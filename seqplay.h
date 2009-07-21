#pragma once

#include "cosecant_api.h"
#include "sequence.h"

class SeqPlay;

struct SeqPlayEvent
{
	enum Type { patternStart, patternStop, timeChange };
	Type m_type;

	SeqPlayEvent(Type type) : m_type(type) {}

	struct PatternStart
	{
		Sequence::Track* track;
		Sequence::Pattern* pattern;
		double pos;
	};

	struct PatternStop
	{
		Sequence::Track* track;
	};

	union
	{
		PatternStart m_patternStart;
		PatternStop m_patternStop;
		CosecantAPI::TimeInfo m_timeChange;
	};
};

typedef QMap<int, SeqPlayEvent> SeqPlayEventMap;

class SeqTrackPlay : public Object
{
public:
	SeqTrackPlay(SeqPlay* sp, const Ptr<Sequence::Track>& track, SeqPlayEventMap& events)
		: m_sp(sp), m_track(track), m_events(events), m_workFromScratch(true)
	{}

	void preWork() { m_events.clear(); }
	void work(int firstframe, int lastframe, bool fromScratch);

	SeqPlayEventMap& m_events;

protected:
	SeqPlay* m_sp;
	Ptr<Sequence::Track> m_track;
	bool m_workFromScratch;

	Sequence::Track::Clips::const_iterator m_iter; // points to the next pattern to play
	Ptr<Sequence::Clip> m_playingClip;
};

class SeqPlay : public QObject
{
	Q_OBJECT

	friend class SeqTrackPlay;

protected:
	SeqPlay(const Ptr<Sequence::Seq>& seq);

	static SingletonPtr<SeqPlay> s_singleton;

public:
	QReadWriteLock_Recursive m_mutex;

	static void initSingleton(const Ptr<Sequence::Seq>& seq) { s_singleton.set(new SeqPlay(seq)); }
	static SeqPlay& get() { return *s_singleton; }

	const CosecantAPI::TimeInfo& getTimeInfo() { return m_timeinfo; }

	bool isPlaying() { return m_playing; }
	double getPlayPos() { return m_playPos; }

	void setPlaying(bool playing);

	void preWork();
	void work(int firstframe, int lastframe, bool fromScratch);

	QHash< Ptr<Machine>, SeqPlayEventMap > m_events;
	QHash< Ptr<Sequence::Track>, Ptr<SeqTrackPlay> > m_trackPlays;

public slots:
	void onInsertTrack(int index, const Ptr<Sequence::Track>& track);
	void onRemoveTrack(int index, const Ptr<Sequence::Track>& track);

protected:
	Ptr<Sequence::Seq> m_seq;
	CosecantAPI::TimeInfo m_timeinfo;
	
	bool m_playing;
	double m_playPos; // in beats

	double m_beatsPerSample;
};
