#pragma once

#include "cosecant_api.h"
#include "sequence.h"
#include "eventstream.h"

class SeqPlay;

class SeqTrackPlay : public Object
{
	Q_OBJECT

public:
	SeqTrackPlay(SeqPlay* sp, const Ptr<Sequence::Track>& track);

	void work(int firstframe, int lastframe, bool fromScratch);

protected slots:
	void onAddClip(const Ptr<Sequence::Clip>& clip);
	void onRemoveClip(const Ptr<Sequence::Clip>& clip);

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

	void work(int firstframe, int lastframe, bool fromScratch);

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
