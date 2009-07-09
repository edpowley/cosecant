#pragma once

#include "sequence.h"

namespace SequenceActions
{
	class ChangeEvents : public QUndoCommand
	{
	public:
		ChangeEvents(const QString& description)
		{
			setText(description);
		}

		void addAddEvent(const Ptr<Sequence::Track>& track, const Ptr<Sequence::Clip>& ev);
		void addRemoveEvent(const Ptr<Sequence::Track>& track, const Ptr<Sequence::Clip>& ev);
		void addReplaceEvent(const Ptr<Sequence::Track>& track,
							const Ptr<Sequence::Clip>& evOld, const Ptr<Sequence::Clip>& evNew);

		virtual void redo();
		virtual void undo();

	protected:
		std::multimap< Ptr<Sequence::Track>, Ptr<Sequence::Clip> > m_eventsAdded, m_eventsRemoved;
		std::set< Ptr<Sequence::Track> > m_tracksAffected;

		QString m_description;
	};

	class InsertEvent : public ChangeEvents
	{
	public:
		InsertEvent(const Ptr<Sequence::Track>& track, const Ptr<Sequence::Clip>& ev);
	};

	class CreatePatternAndInsertEvent : public InsertEvent
	{
	public:
		CreatePatternAndInsertEvent(const Ptr<Sequence::Track>& track, const Ptr<Sequence::Clip>& ev);

		virtual void redo();
		virtual void undo();

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

		virtual void redo();
		virtual void undo();

	protected:
		Ptr<Sequence::Pattern> m_pattern;
		QUndoCommand* m_actualUndoable;
	};
};
