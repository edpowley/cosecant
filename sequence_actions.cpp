#include "stdafx.h"
#include "common.h"
#include "sequence_actions.h"
#include "machine.h"
#include "song.h"

using namespace SequenceActions;

////////////////////////////////////////////////////////////////////////////////////

InsertEvent::InsertEvent(const Ptr<Sequence::Track>& track, const Ptr<Sequence::Event>& ev)
: ChangeEvents("insert sequence event")
{
	addAddEvent(track, ev);

	// If ev's start time is in an event, truncate that event
	Ptr<Sequence::Event> truncEvent = track->getEventAtTime(ev->m_startTime);
	if (truncEvent)
	{
		addRemoveEvent(track, truncEvent);

		// If we're at the start of an event, this deletes it entirely.
		// Otherwise...
		if (truncEvent->m_startTime < ev->m_startTime)
		{
			Ptr<Sequence::Event> truncatedEvent = new Sequence::Event(*truncEvent);
			truncatedEvent->m_end = truncEvent->m_begin + (ev->m_startTime - truncEvent->m_startTime);
			addAddEvent(track, truncatedEvent);
		}
	}

	// If there is an event soon after the one we're inserting, then that limits the length
	Ptr<Sequence::Event> nextEvent = track->getNextEvent(ev->m_startTime);
	if (nextEvent)
	{
		ev->m_end = min(ev->m_end, ev->m_begin + (nextEvent->m_startTime - ev->m_startTime));
	}
}

CreatePatternAndInsertEvent::CreatePatternAndInsertEvent(const Ptr<Sequence::Track>& track, const Ptr<Sequence::Event>& ev)
: InsertEvent(track, ev), m_mac(track->m_mac), m_pattern(ev->m_pattern)
{
}

void CreatePatternAndInsertEvent::redo()
{
	m_mac->addPattern(m_pattern);
	InsertEvent::redo();
}

void CreatePatternAndInsertEvent::undo()
{
	InsertEvent::undo();
	m_mac->removePattern(m_pattern);
}

ClearTrackRange::ClearTrackRange(const Ptr<Sequence::Track>& track, double begin, double end, bool deleteIfStartsHere)
: ChangeEvents("delete sequence events")
{
	BOOST_FOREACH(const Ptr<Sequence::Event>& ev, track->m_events)
	{
		if (ev->m_startTime >= end) break;

		if (ev->m_startTime >= begin && ev->m_startTime < end)
		{
			addRemoveEvent(track, ev);
			if (!deleteIfStartsHere)
			{
				// todo
			}
		}
		else if (ev->getEndTime() > begin)
		{
			addRemoveEvent(track, ev);
			Ptr<Sequence::Event> truncatedEvent = new Sequence::Event(*ev);
			truncatedEvent->m_end = ev->m_begin + (begin - ev->m_startTime);
			addAddEvent(track, truncatedEvent);
		}
	}
}

ChangePatternLength::ChangePatternLength(const Ptr<Sequence::Pattern>& pattern, double newlength)
: ChangeEvents("change pattern length"), m_pattern(pattern)
{
	m_actualUndoable = m_pattern->createUndoableForLengthChange(newlength);

	BOOST_FOREACH(const Ptr<Sequence::Track>& track, Song::get().m_sequence->m_tracks)
	{
		if (track->m_mac.c_ptr() != m_pattern->m_mac) continue;

		BOOST_FOREACH(const Ptr<Sequence::Event>& ev, track->m_events)
		{
			if (ev->m_pattern != m_pattern) continue;

			// If the end point is currently the end of the pattern, or if the end point is beyond the new length
			if (ev->m_end == m_pattern->getLength() || ev->m_end > newlength)
			{
				Ptr<Sequence::Event> newev = new Sequence::Event(*ev);
				newev->m_end = newlength;

				if (newlength > m_pattern->getLength()) // need to check for overlap with next event
				{
					Ptr<Sequence::Event> nextev = track->getNextEvent(newev->m_startTime);
					if (nextev && nextev->m_startTime < newev->getEndTime())
						newev->m_end = newev->m_begin + (nextev->m_startTime - newev->m_startTime);
				}

				addReplaceEvent(track, ev, newev);
			}
		}
	}
}

void ChangePatternLength::redo()
{
	m_actualUndoable->redo();
	ChangeEvents::redo();
	Song::get().m_sequence->trigger_signalMachinePatternsChange(m_pattern->m_mac);
}

void ChangePatternLength::undo()
{
	ChangeEvents::undo();
	m_actualUndoable->undo();
	Song::get().m_sequence->trigger_signalMachinePatternsChange(m_pattern->m_mac);
}

////////////////////////////////////////////////////////////////////////////////////

typedef std::pair< Ptr<Sequence::Track>, Ptr<Sequence::Event> > TrackEventPair;

void ChangeEvents::addAddEvent(const Ptr<Sequence::Track>& track, const Ptr<Sequence::Event>& ev)
{
	m_eventsAdded.insert(std::make_pair(track, ev));
	m_tracksAffected.insert(track);
}

void ChangeEvents::addRemoveEvent(const Ptr<Sequence::Track>& track, const Ptr<Sequence::Event>& ev)
{
	m_eventsRemoved.insert(std::make_pair(track, ev));
	m_tracksAffected.insert(track);
}

void ChangeEvents::addReplaceEvent(const Ptr<Sequence::Track>& track,
								   const Ptr<Sequence::Event>& evOld, const Ptr<Sequence::Event>& evNew)
{
	addRemoveEvent(track, evOld);
	addAddEvent(track, evNew);
	m_tracksAffected.insert(track);
}

void ChangeEvents::redo()
{
	boost::unique_lock<boost::shared_mutex> lock(Song::get().m_sequence->m_mutex);

	BOOST_FOREACH(const TrackEventPair& te, m_eventsRemoved)
	{
		te.first->m_events.erase(te.second);
	}

	BOOST_FOREACH(const TrackEventPair& te, m_eventsAdded)
	{
		te.first->m_events.insert(te.second);
	}

	BOOST_FOREACH(const Ptr<Sequence::Track>& track, m_tracksAffected)
	{
		track->trigger_signalChange();
	}
}

void ChangeEvents::undo()
{
	boost::unique_lock<boost::shared_mutex> lock(Song::get().m_sequence->m_mutex);

	BOOST_FOREACH(const TrackEventPair& te, m_eventsAdded)
	{
		te.first->m_events.erase(te.second);
	}

	BOOST_FOREACH(const TrackEventPair& te, m_eventsRemoved)
	{
		te.first->m_events.insert(te.second);
	}

	BOOST_FOREACH(const Ptr<Sequence::Track>& track, m_tracksAffected)
	{
		track->trigger_signalChange();
	}
}
