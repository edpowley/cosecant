#include "stdafx.h"
#include "common.h"
#include "workunit.h"
#include "workqueue.h"
#include "perfclock.h"
#include "song.h"
#include "seqplay.h"
#include "timeunit_convert.h"

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
	m_inpins(machine->m_inpins), m_outpins(machine->m_outpins)
{
	m_eventWorkBuffer = new WorkBuffer::Events;

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
	m_workContext.in  = m_inPinBuffer;
	m_workContext.out = m_outPinBuffer;
	m_eventPinBuffer = m_eventWorkBuffer->getPinBuffer();
	m_workContext.ev = &m_eventPinBuffer;

	for (size_t pin=0; pin<m_inpins.size(); pin++)
	{
		ParamPin* parampin = dynamic_cast<ParamPin*>(m_inpins[pin].c_ptr());
		if (parampin)
		{
			ParamPinBuf ppb;
			ppb.param = parampin->getParam();
			ppb.tag = ppb.param->getTag();
			ppb.buf = dynamic_cast<WorkBuffer::ParamControl*>(m_inWorkBuffer[pin].c_ptr());

			Parameter::Time* ptime = dynamic_cast<Parameter::Time*>(ppb.param.c_ptr());
			if (ptime)
			{
				ppb.doTimeUnitConversion = true;
				ppb.fromTimeUnit = parampin->getTimeUnit();
				ppb.toTimeUnit = ptime->getInternalUnit();
			}
			else
				ppb.doTimeUnitConversion = false;

			m_paramPinBufs.push_back(ppb);
		}
	}
}

void WorkMachine::work(int firstframe, int lastframe)
{
	if (m_machine->m_dead) return;

	try
	{
		PerfClockAutoCount clock(&m_machine->m_perfCount);

		MutexLockerWithTimeout lock(&m_machine->m_mutex, 1000);

		m_eventWorkBuffer->clearAll();

		{
			QMutexLocker paramChangeLock(&m_machine->m_paramChangesMutex);

			typedef std::pair<ParamTag, double> parampair;
			BOOST_FOREACH(const parampair& p, m_machine->m_paramChanges)
			{
				StreamEvent ev;
				ev.type = StreamEventType::paramChange;
				ev.time = firstframe;
				ev.paramChange.tag = p.first;
				ev.paramChange.value = p.second;
				m_eventWorkBuffer->m_data.insert(ev);
			}

			// Clear parameter changes
			m_machine->m_paramChanges.clear();
		}

		BOOST_FOREACH(ParamPinBuf& ppb, m_paramPinBufs)
		{
			std::map<int, double>::const_iterator iter = ppb.buf->m_data.lower_bound(firstframe);
			std::map<int, double>::const_iterator enditer = ppb.buf->m_data.lower_bound(lastframe);
			for (; iter != enditer; ++iter)
			{
				double v = iter->second;

				if (ppb.doTimeUnitConversion)
					v = ConvertTimeUnits(ppb.fromTimeUnit, ppb.toTimeUnit, v);

				v = ppb.param->sanitise(v);

				StreamEvent ev;
				ev.type = StreamEventType::paramChange;
				ev.time = iter->first;
				ev.paramChange.tag = ppb.tag;
				ev.paramChange.value = v;
				m_eventWorkBuffer->m_data.insert(ev);
				ppb.param->setState(v);
			}
		}

		// Find break points (points where something changes)
		std::set<int> breakpoints;
		breakpoints.insert(lastframe);
		m_eventWorkBuffer->getEventBreakPoints(breakpoints);

		for (size_t pin=0; pin<m_inpins.size(); pin++)
		{
			if (m_inpins[pin]->getFlags() & PinFlags::breakOnEvent)
			{
				m_inWorkBuffer[pin]->getEventBreakPoints(breakpoints);
			}
		}

		static const EventStream emptySpem;
		const EventStream& spem = SeqPlay::get().m_events.value(m_machine, emptySpem);

		for (EventStream::const_iterator iter = spem.begin(); iter != spem.end(); ++iter)
		{
			breakpoints.insert(iter->time);
		}

		// Do the work
		int frame = firstframe;
		EventStream::const_iterator spemIter = spem.begin();
		BOOST_FOREACH(int breakpoint, breakpoints)
		{
			// Seq play events
			for (; spemIter != spem.end() && spemIter->time == frame; ++spemIter)
			{
				const StreamEvent& spe = *spemIter;
				switch (spe.type)
				{
				case StreamEventType::patternPlay:
					spe.pattern.hostPattern->play(spe.pattern.track, spe.pattern.pos);
					break;

				case StreamEventType::patternStop:
					spe.pattern.hostPattern->stop(spe.pattern.track);
					break;
				}
			}

			if (breakpoint != frame)
			{
				m_workContext.firstframe = frame;
				m_workContext.lastframe = breakpoint;
				m_machine->work(&m_workContext);
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
