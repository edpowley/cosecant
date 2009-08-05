#include "stdafx.h"
#include "common.h"
#include "workunit.h"
#include "workqueue.h"
#include "eventlist.h"
#include "perfclock.h"
#include "song.h"
#include "seqplay.h"

using namespace WorkUnit;

void Base::addDependent(const Ptr<Base>& other)
{
	// It doesn't actually matter if a dependent is added more than once, because
	// other->m_depsToGo will be decremented by the right amount anyway. In other words,
	// if a dependent is listed twice, then its depsAll count will be one bigger, but its
	// depsToGo count will be decremented twice so everything works out OK.
	m_dependents.push_back(other);
	other->m_depsAll ++;
}

void Base::depMM()
{
	m_depsToGo--;
	if (m_depsToGo == 0)
	{
		m_queue->m_ready.push_back(this);
	}
}

void Base::go(int numframes)
{
	work(0, numframes);

	{
		QMutexLocker lock(&m_queue->m_mutex);

		if (!m_dependents.empty())
		{
			for (std::vector< Ptr<Base> >::iterator i = m_dependents.begin();
				i != m_dependents.end(); ++i)
			{
				(*i)->depMM();
			}
		}

		m_queue->m_toWork --;
	}
}

/////////////////////////////////////////////////////////////////////////////

WorkMachine::WorkMachine(WorkQueue* q, const Ptr<Machine>& machine)
:	Base(q), m_machine(machine), m_inPinBuffer(NULL), m_outPinBuffer(NULL),
	m_inpins(machine->m_inpins), m_outpins(machine->m_outpins), m_noteTriggerPin(machine->m_noteTriggerPin)
{
	BOOST_FOREACH(Ptr<Sequence::Track>& track, Song::get().m_sequence->m_tracks)
	{
		if (track->m_mac == m_machine)
			m_seqTracks.push_back(track);
	}
}

WorkMachine::~WorkMachine()
{
	if (m_inPinBuffer)  delete[] m_inPinBuffer;
	if (m_outPinBuffer) delete[] m_outPinBuffer;
}

void WorkMachine::updatePinBuffers(PinBuffer*& pinBuffers,
								   const std::vector< Ptr<WorkBuffer::Base> >& workBuffers)
{
	if (pinBuffers) delete[] pinBuffers;
	if (workBuffers.empty())
	{
		pinBuffers = NULL;
	}
	else
	{
		pinBuffers = new PinBuffer[workBuffers.size()];
		PinBuffer* pb = pinBuffers;

		BOOST_FOREACH(const Ptr<WorkBuffer::Base>& wb, workBuffers)
		{
			*pb = wb->getPinBuffer();
			++pb;
		}
	}
}

void WorkMachine::updatePinBuffers()
{
	updatePinBuffers(m_inPinBuffer,  m_inWorkBuffer);
	updatePinBuffers(m_outPinBuffer, m_outWorkBuffer);

	for (size_t pin=0; pin<m_inpins.size(); pin++)
	{
		if (m_inpins[pin]->m_isParamPin)
		{
			ParamPinBuf ppb;
			ppb.tag = m_inpins[pin]->m_paramTag;
			ppb.buf = dynamic_cast<WorkBuffer::ParamControl*>(m_inWorkBuffer[pin].c_ptr());
			m_paramPinBufs.push_back(ppb);
		}
		else if (m_inpins[pin] == m_noteTriggerPin)
		{
			m_notePinBuf.buf = dynamic_cast<WorkBuffer::SequenceEvents*>(m_inWorkBuffer[pin].c_ptr());
		}
	}
}

void WorkMachine::sendParamChanges()
{
	QMutexLocker paramChangeLock(&m_machine->m_paramChangesMutex);

	typedef std::pair<ParamTag, double> parampair;
	BOOST_FOREACH(const parampair& p, m_machine->m_paramChanges)
	{
		m_machine->changeParam(p.first, p.second);
	}

	// Clear parameter changes
	m_machine->m_paramChanges.clear();
}

void WorkMachine::work(int firstframe, int lastframe)
{
	if (m_machine->m_dead) return;

	try
	{
		PerfClockAutoCount clock(&m_machine->m_perfCount);

		MutexLockerWithTimeout lock(&m_machine->m_mutex, 1000);

		sendParamChanges();

		// Find break points (points where something changes)
		std::set<int> breakpoints;
		breakpoints.insert(lastframe);

		BOOST_FOREACH(ParamPinBuf& ppb, m_paramPinBufs)
		{
			ppb.iter = ppb.buf->m_data.lower_bound(firstframe);
			ppb.enditer = ppb.buf->m_data.lower_bound(lastframe);

			for (std::map<int, double>::const_iterator pi = ppb.iter; pi != ppb.enditer; ++pi)
			{
				breakpoints.insert(pi->first);
			}
		}

		if (m_notePinBuf.buf)
		{
			m_notePinBuf.iter = m_notePinBuf.buf->m_data.lower_bound(firstframe);
			m_notePinBuf.enditer = m_notePinBuf.buf->m_data.lower_bound(lastframe);

			for (WorkBuffer::SequenceEvents::EventMap::const_iterator ni = m_notePinBuf.iter;
				ni != m_notePinBuf.enditer; ++ni)
			{
				breakpoints.insert(ni->first);
			}
		}

		static const SeqPlayEventMap emptySpem;
		const SeqPlayEventMap& spem = SeqPlay::get().m_events.value(m_machine, emptySpem);

		for (SeqPlayEventMap::const_iterator iter = spem.begin(); iter != spem.end(); ++iter)
		{
			breakpoints.insert(iter.key());
		}

		// Do the work
		int frame = firstframe;
		SeqPlayEventMap::const_iterator spemIter = spem.begin();
		BOOST_FOREACH(int breakpoint, breakpoints)
		{
			// Param pins
			BOOST_FOREACH(ParamPinBuf& ppb, m_paramPinBufs)
			{
				if (ppb.iter != ppb.enditer && ppb.iter->first == frame)
				{
					m_machine->changeParam(ppb.tag, ppb.iter->second);
					++ ppb.iter;
				}
			}

			// Seq play events
			for (; spemIter != spem.end() && spemIter.key() == frame; ++spemIter)
			{
				const SeqPlayEvent& spe = spemIter.value();
				switch (spe.m_type)
				{
				case SeqPlayEvent::patternStart:
					spe.m_patternStart.pattern->play(spe.m_patternStart.track, spe.m_patternStart.pos);
					break;

				case SeqPlayEvent::patternStop:
					spe.m_patternStop.pattern->stop(spe.m_patternStop.track);
					break;
				}
			}

			// Notes
#if 0
			NotePinBuf& npb = m_notePinBuf;
			if (npb.buf)
			{
				for (; npb.iter != npb.enditer && npb.iter->first == frame; ++ npb.iter)
				{
					// assert(npb.iter->second.m_type == SequenceEvent::note);
					SequenceEvent::Note* nev = dynamic_cast<SequenceEvent::Note*>(npb.iter->second.c_ptr());
					const NoteEvent& ev = nev->m_data;
					std::map<void*, void*>::iterator idIter = m_machine->m_noteIdMap.find(ev.id);

					if (ev.vel > 0.0) // note on
					{
						if (idIter != m_machine->m_noteIdMap.end())
							m_machine->getMi()->noteOff(idIter->second);

						m_machine->m_noteIdMap[ev.id] = m_machine->getMi()->noteOn(ev.note, ev.vel);
					}
					else // note off
					{
						if (idIter != m_machine->m_noteIdMap.end())
						{
							m_machine->getMi()->noteOff(idIter->second);
							m_machine->m_noteIdMap.erase(idIter);
						}
					}
				}
			}
#endif

			if (breakpoint != frame)
			{
				m_machine->work(m_inPinBuffer, m_outPinBuffer, frame, breakpoint);
			}

			frame = breakpoint;
		}
	}
	catch (const std::exception& err)
	{
		m_machine->m_dead = true;
		m_machine->m_deadWhy = QString("Unhandled exception while working:\n") + err.what();
	}
}

////////////////////////////////////////////////////////////////////////////////

void FanIn::work(int firstframe, int lastframe)
{
	if (m_inbufs.empty())
		m_outbuf->clear(firstframe, lastframe);
	else
	{
		m_outbuf->copy(m_inbufs[0], firstframe, lastframe);
		for (std::vector< Ptr<WorkBuffer::Base> >::iterator iter = m_inbufs.begin() + 1;
			iter != m_inbufs.end(); ++iter)
		{
			m_outbuf->mix(*iter, firstframe, lastframe);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

void FeedbackRead::work(int firstframe, int lastframe)
{
	m_line->read(m_buf, firstframe, lastframe);
}

void FeedbackWrite::work(int firstframe, int lastframe)
{
	m_line->write(m_buf, firstframe, lastframe);
}

////////////////////////////////////////////////////////////////////////////////

void Chain::work(int firstframe, int lastframe)
{
	for (int i = firstframe; i < lastframe; i += m_maxFramesPerWork)
	{
		int j = min(lastframe, i + m_maxFramesPerWork);

		BOOST_FOREACH(const Ptr<Base>& u, m_units)
		{
			u->work(i, j);
		}
	}
}
