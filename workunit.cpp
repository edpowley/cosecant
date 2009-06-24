#include "stdafx.h"
#include "common.h"
#include "workunit.h"
#include "workqueue.h"
#include "eventlist.h"
#include "perfclock.h"
#include "song.h"

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
		boost::unique_lock<boost::shared_mutex> lock(m_queue->m_mutex);

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
	boost::unique_lock<boost::mutex> paramChangeLock(m_machine->m_paramChangesMutex);

	typedef std::pair<ParamTag, ParamValue> parampair;
	BOOST_FOREACH(const parampair& p, m_machine->m_paramChanges)
	{
		m_machine->changeParam(p.first, p.second);
	}

	// Clear parameter changes
	m_machine->m_paramChanges.clear();
}

void WorkMachine::updateSequenceEvents(int firstframe, int lastframe)
{
	m_sequenceEvents.clear();

	if (m_queue->m_playing)
	{
		if (m_queue->m_shouldUpdateSequenceFromScratch)
			m_machine->m_playingEvents.clear();

		double endPlayPos = m_queue->m_playPos + m_queue->m_ticksPerFrame * (lastframe - firstframe);
		BOOST_FOREACH(const Ptr<Sequence::Track>& track, m_seqTracks)
		{
			BOOST_FOREACH(const Ptr<Sequence::Event>& sev, track->m_events)
			{
				if (sev->getEndTime() < m_queue->m_playPos) continue;
				if (sev->m_startTime >= endPlayPos) break;

				if (m_queue->m_shouldUpdateSequenceFromScratch)
				{
					m_machine->m_playingEvents.push_back(
						Machine::EventPlayRec(sev, sev->m_begin + (m_queue->m_playPos - sev->m_startTime))
					);
				}

				double t = sev->m_startTime;
				if (t >= m_queue->m_playPos && t < endPlayPos)
				{
					int it = firstframe + static_cast<int>( (t - m_queue->m_playPos) * m_queue->m_framesPerTick );
					m_sequenceEvents.insert(std::make_pair(it, new SequenceEvent::Start(track, sev, sev->m_begin)));
					//breakpoints.insert(it);
				}

				t = sev->getEndTime();
				if (t >= m_queue->m_playPos && t < endPlayPos)
				{
					int it = firstframe + static_cast<int>( (t - m_queue->m_playPos) * m_queue->m_framesPerTick );
					m_sequenceEvents.insert(std::make_pair(it, new SequenceEvent::Stop(track, sev)));
					//breakpoints.insert(it);
				}
			}
		}
	}
}

void WorkMachine::work(int firstframe, int lastframe)
{
	if (m_machine->m_dead) return;

	try
	{
		PerfClockAutoCount clock(&m_machine->m_perfCount);

		TimeoutLock<boost::recursive_timed_mutex> lock(m_machine->m_mutex, 1000);
		boost::unique_lock<boost::mutex> lockb(m_machine->m_playingPatternsMutex);

		sendParamChanges();

		// Find break points (points where param pins change)
		std::set<int> breakpoints;
		breakpoints.insert(lastframe);

		BOOST_FOREACH(ParamPinBuf& ppb, m_paramPinBufs)
		{
			ppb.iter = ppb.buf->m_data.lower_bound(firstframe);
			ppb.enditer = ppb.buf->m_data.lower_bound(lastframe);

			for (std::map<int, ParamValue>::const_iterator pi = ppb.iter; pi != ppb.enditer; ++pi)
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

		updateSequenceEvents(firstframe, lastframe);
		for (SequenceEventMap::const_iterator sei = m_sequenceEvents.begin(); sei != m_sequenceEvents.end(); ++sei)
		{
			breakpoints.insert(sei->first);
		}

		// Do the work
		int frame = firstframe;
		double playPos = m_queue->m_playPos;
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

			// Notes
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
							m_machine->noteOff(idIter->second);

						m_machine->m_noteIdMap[ev.id] = m_machine->noteOn(ev.note, ev.vel);
					}
					else // note off
					{
						if (idIter != m_machine->m_noteIdMap.end())
						{
							m_machine->noteOff(idIter->second);
							m_machine->m_noteIdMap.erase(idIter);
						}
					}
				}
			}

			// Sequence play events
			{
				std::pair<SequenceEventMap::iterator, SequenceEventMap::iterator> spei
					= m_sequenceEvents.equal_range(frame);
				for (SequenceEventMap::iterator i = spei.first; i != spei.second; ++i)
				{
					i->second->work(m_machine);
				}
			}

			if (breakpoint != frame)
			{
				m_machine->work(m_inPinBuffer, m_outPinBuffer, frame, breakpoint);
			}

			double posIncrement = (breakpoint - frame) * m_queue->m_ticksPerFrame;
			playPos += posIncrement;
			BOOST_FOREACH(Machine::EventPlayRec& epr, m_machine->m_playingEvents)
			{
				epr.m_pos += posIncrement;
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