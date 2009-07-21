#include "stdafx.h"
#include "common.h"
#include "seqplay.h"
#include "song.h"
#include "sequence.h"

SingletonPtr<SeqPlay> SeqPlay::s_singleton;

SeqPlay::SeqPlay(const Ptr<Sequence::Seq>& seq)
: m_seq(seq), m_playing(false), m_playPos(0)
{
	m_timeinfo.beatsPerSecond = 120.0 / 60.0;
	m_timeinfo.beatsPerBar = 4;
	m_timeinfo.beatsPerWholeNote = 4;
	m_timeinfo.barsPerSmallGrid = 4;
	m_timeinfo.smallGridsPerLargeGrid = 4;
	m_timeinfo.samplesPerSecond = 44100;

	m_beatsPerSample = m_timeinfo.beatsPerSecond / m_timeinfo.samplesPerSecond;

	foreach(const Ptr<Sequence::Track>& track, m_seq->m_tracks)
	{
		Ptr<SeqTrackPlay> stp = new SeqTrackPlay(this, track, m_events[track->m_mac]);
		m_trackPlays.insert(track, stp);
	}

	connect(m_seq, SIGNAL(signalInsertTrack(int, const Ptr<Sequence::Track>&)),
		this, SLOT(onInsertTrack(int, const Ptr<Sequence::Track>&)) );
	connect(m_seq, SIGNAL(signalRemoveTrack(int, const Ptr<Sequence::Track>&)),
		this, SLOT(onRemoveTrack(int, const Ptr<Sequence::Track>&)) );
}

void SeqPlay::onInsertTrack(int index, const Ptr<Sequence::Track>& track)
{
	QWriteLocker lock(&m_mutex);

	Ptr<SeqTrackPlay> stp = new SeqTrackPlay(this, track, m_events[track->m_mac]);
	m_trackPlays.insert(track, stp);	
}

void SeqPlay::onRemoveTrack(int index, const Ptr<Sequence::Track>& track)
{
	QWriteLocker lock(&m_mutex);

	m_trackPlays.remove(track);
}

void SeqPlay::setPlaying(bool playing)
{
	m_playing = playing;
}

void SeqPlay::preWork()
{
	foreach(const Ptr<SeqTrackPlay>& stp, m_trackPlays)
	{
		stp->preWork();
	}
}

void SeqPlay::work(int firstframe, int lastframe, bool fromScratch)
{
	if (!m_playing) return;
	if (firstframe >= lastframe) return;

	QReadLocker(&m_seq->m_mutex);

	double nextpos = m_playPos + (lastframe - firstframe) * m_beatsPerSample;
	
	// Check for looping
	if (nextpos >= m_seq->m_loopEnd)
	{
		// Work the bit before the loop, then the bit after the loop
		int loopframe = firstframe + (int)floor( (m_seq->m_loopEnd - m_playPos) / m_beatsPerSample );
		work(firstframe, loopframe, fromScratch);
		m_playPos -= (m_seq->m_loopEnd - m_seq->m_loopStart);
		work(loopframe, lastframe, true);
		return;
	}

	foreach(const Ptr<SeqTrackPlay>& stp, m_trackPlays)
	{
		stp->work(firstframe, lastframe, fromScratch);
	}

	m_playPos = nextpos;
}

void SeqTrackPlay::work(int firstframe, int lastframe, bool fromScratch)
{
	double playpos = m_sp->m_playPos;
	double nextpos = playpos + (lastframe - firstframe) * m_sp->m_beatsPerSample;

	if (fromScratch || m_workFromScratch)
	{
		m_iter = m_track->m_clips.lowerBound(playpos);
		if (m_playingClip)
		{
			SeqPlayEvent spe(SeqPlayEvent::patternStop);
			spe.m_patternStop.track = m_track;
			m_events.insert(firstframe, spe);
		}
		m_playingClip = NULL;

		m_workFromScratch = false;
	}

	while (firstframe < lastframe)
	{
		if (m_playingClip && m_playingClip->getEndTime() < nextpos)
		{
			int f = firstframe + (int)floor( (m_playingClip->getEndTime() - playpos) / m_sp->m_beatsPerSample );
			SeqPlayEvent spe(SeqPlayEvent::patternStop);
			spe.m_patternStop.track = m_track;
			m_events.insert(f, spe);
			m_playingClip = NULL;

			playpos += (f - firstframe) * m_sp->m_beatsPerSample;
			firstframe = f;
		}
		else if (m_iter != m_track->m_clips.end() && m_iter.key() < nextpos)
		{
			int f = firstframe + (int)floor( (m_iter.key() - playpos) / m_sp->m_beatsPerSample );
			m_playingClip = m_iter.value();
			SeqPlayEvent spe(SeqPlayEvent::patternStart);
			spe.m_patternStart.track = m_track;
			spe.m_patternStart.pattern = m_playingClip->m_pattern;
			spe.m_patternStart.pos = m_playingClip->m_begin + (playpos - m_iter.key());
			m_events.insert(f, spe);

			playpos += (f - firstframe) * m_sp->m_beatsPerSample;
			firstframe = f;
			++ m_iter;
		}
		else
		{
			break;
		}
	}
}
