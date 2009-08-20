#pragma once

#include "sequence.h"

class SpatternMachine;
class SpatternPlayer;

namespace Spattern
{
	class Pattern;

	class Note : public Object
	{
	public:
		Note(Pattern* pattern, double start, double length, double note);

		double getStart() { return m_start; }
		double getLength() { return m_length; }
		double getEnd() { return m_start + m_length; }
		double getNote() { return m_note; }

	protected:
		Pattern* m_pattern;
		double m_start, m_length, m_note;
	};

	class Pattern : public Sequence::Pattern
	{
		Q_OBJECT
		
		friend SpatternPlayer;

	public:
		Pattern(SpatternMachine* mac, double length);

		virtual void play(Sequence::Track* track, double startpos);
		virtual void stop(Sequence::Track* track);

		void addNote(const Ptr<Note>& note);
		void removeNote(const Ptr<Note>& note);
		QMultiMap< double, Ptr<Note> > getNotes() { return m_notes; }

		virtual void load(SongLoadContext& ctx, const QDomElement& el);
		virtual QDomElement save(QDomDocument& doc);

	signals:
		void signalNoteAdded(const Ptr<Spattern::Note>& note);
		void signalNoteRemoved(const Ptr<Spattern::Note>& note);

	protected:
		SpatternMachine* m_mac;

		QMultiMap< double, Ptr<Note> > m_notes;
	};
};
