#pragma once

#include "builtinmachines.h"
#include "spattern.h"
#include "workbuffer.h"

class SpatternPlayer : public Object
{
public:
	SpatternPlayer(const Ptr<Spattern::Pattern>& pattern, double startpos);

	void work(const Ptr<WorkBuffer::Events>& outbuf, int firstframe, int lastframe, double beatsPerFrame);
	
	QList< Ptr<Spattern::Note> > getPlayingNotes() { return m_playingNotes; }
	Ptr<Spattern::Pattern> getPattern() { return m_pattern; }

protected:
	Ptr<Spattern::Pattern> m_pattern;
	double m_pos;
	QMultiMap< double, Ptr<Spattern::Note> >::const_iterator m_iter, m_enditer;
	void resetIter();
	QList< Ptr<Spattern::Note> > m_playingNotes;
};

class SpatternMachine : public BuiltinMachine
{
public:
	SpatternMachine();

	virtual void work(const WorkContext* ctx);

	void playPattern(Sequence::Track* track, Spattern::Pattern* patt, double startpos);
	void stopPattern(Sequence::Track* track, Spattern::Pattern* patt);

	virtual QWidget* createPatternEditorWidget(const Ptr<Sequence::Pattern>& pattern);

protected:
	virtual void initInfo();
	virtual void initImpl();

	virtual Ptr<Sequence::Pattern> createPatternImpl(double length);

	QHash< Ptr<Sequence::Track>, Ptr<SpatternPlayer> > m_players;
	QList< Ptr<Spattern::Note> > m_stoppingNotes;
};

