#pragma once

#include "sequence.h"
#include "undoredo.h"

namespace SequenceActions
{
	class ChangeEvents : public Undoable
	{
	public:
		ChangeEvents(const QString& description)
			: m_description(description) {}

		void addAddEvent(const Ptr<Sequence::Track>& track, const Ptr<Sequence::Event>& ev);
		void addRemoveEvent(const Ptr<Sequence::Track>& track, const Ptr<Sequence::Event>& ev);
		void addReplaceEvent(const Ptr<Sequence::Track>& track,
							const Ptr<Sequence::Event>& evOld, const Ptr<Sequence::Event>& evNew);

		virtual QString describe() { return m_description; }
		virtual bool operator()();
		virtual bool undo();

	protected:
		std::multimap< Ptr<Sequence::Track>, Ptr<Sequence::Event> > m_eventsAdded, m_eventsRemoved;
		std::set< Ptr<Sequence::Track> > m_tracksAffected;

		QString m_description;
	};

	class InsertEvent : public ChangeEvents
	{
	public:
		InsertEvent(const Ptr<Sequence::Track>& track, const Ptr<Sequence::Event>& ev);
	};

	class CreatePatternAndInsertEvent : public InsertEvent
	{
	public:
		CreatePatternAndInsertEvent(const Ptr<Sequence::Track>& track, const Ptr<Sequence::Event>& ev);

		virtual bool operator()();
		virtual bool undo();

	protected:
		Ptr<Machine> m_mac;
		Ptr<Sequence::Pattern> m_pattern;
	};

	class ClearTrackRange : public ChangeEvents
	{
	public:
		ClearTrackRange(const Ptr<Sequence::Track>& track, double begin, double end, bool deleteIfStartsHere);
	};

	class ChangePatternLength : public ChangeEvents
	{
	public:
		ChangePatternLength(const Ptr<Sequence::Pattern>& pattern, double newlength);

		virtual bool operator()();
		virtual bool undo();

	protected:
		Ptr<Sequence::Pattern> m_pattern;
		Ptr<Undoable> m_actualUndoable;
	};
};
