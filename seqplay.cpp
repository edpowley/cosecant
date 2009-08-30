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

#ifdef _DEBUG
	foreach(const EventStream& spem, m_events)
	{
		if (!spem.empty())
		{
			qDebug() << "Seqplay eventstream is not empty";
			break;
		}
	}
#endif

	m_playPos = nextpos;
}

///////////////////////////////////////////////////////////////////////////

SeqTrackPlay::SeqTrackPlay(SeqPlay* sp, const Ptr<Sequence::Track>& track, EventStream& events)
: m_sp(sp), m_track(track), m_events(events), m_workFromScratch(true)
{
	connect( m_track, SIGNAL(signalAddClip(const Ptr<Sequence::Clip>&)),
		this, SLOT(onAddClip(const Ptr<Sequence::Clip>&)) );
	connect( m_track, SIGNAL(signalRemoveClip(const Ptr<Sequence::Clip>&)),
		this, SLOT(onRemoveClip(const Ptr<Sequence::Clip>&)) );
}

void SeqTrackPlay::onAddClip(const Ptr<Sequence::Clip>& clip)
{
	QWriteLocker(&m_sp->m_mutex);

	if (m_sp->m_playPos >= clip->m_startTime && m_sp->m_playPos < clip->getEndTime())
	{
		m_workFromScratch = true;
	}
	else
	{
		QReadLocker(&m_sp->m_seq->m_mutex);
		m_iter = m_track->getClips().lowerBound(m_sp->m_playPos);
	}
}

void SeqTrackPlay::onRemoveClip(const Ptr<Sequence::Clip>& clip)
{
	QWriteLocker(&m_sp->m_mutex);

	if (m_playingClip == clip)
	{
		StreamEvent spe;
		spe.type = StreamEventType::patternStop;
		spe.pattern.track = m_track;
		spe.pattern.hostPattern = m_playingClip->m_pattern;
		m_pendingEvents.append(spe);
		m_playingClip = NULL;
	}

	QReadLocker(&m_sp->m_seq->m_mutex);
	m_iter = m_track->getClips().lowerBound(m_sp->m_playPos);
}

void SeqTrackPlay::preWork(int firstframe)
{
	m_events.clear();
	foreach(const StreamEvent& spe, m_pendingEvents)
	{
		StreamEvent ev(spe);
		ev.time = firstframe;
		m_events.insert(ev);
	}
	m_pendingEvents.clear();
}

void SeqTrackPlay::work(int firstframe, int lastframe, bool fromScratch)
{
	double playpos = m_sp->m_playPos;
	double nextpos = playpos + (lastframe - firstframe) * m_sp->m_beatsPerSample;

	if (fromScratch || m_workFromScratch)
	{
		m_iter = m_track->getClips().lowerBound(playpos);
		if (m_iter != m_track->getClips().begin()) -- m_iter;
		if (m_playingClip)
		{
			StreamEvent spe;
			spe.type = StreamEventType::patternStop;
			spe.time = firstframe;
			spe.pattern.track = m_track;
			spe.pattern.hostPattern = m_playingClip->m_pattern;
			m_events.insert(spe);
		}
		m_playingClip = NULL;

		m_workFromScratch = false;
	}

	while (firstframe < lastframe)
	{
		if (m_playingClip && m_playingClip->getEndTime() < nextpos)
		{
			int f = firstframe + (int)floor( (m_playingClip->getEndTime() - playpos) / m_sp->m_beatsPerSample );
			StreamEvent spe;
			spe.type = StreamEventType::patternStop;
			spe.time = f;
			spe.pattern.track = m_track;
			spe.pattern.hostPattern = m_playingClip->m_pattern;
			m_events.insert(spe);
			m_playingClip = NULL;

			playpos += (f - firstframe) * m_sp->m_beatsPerSample;
			firstframe = f;
		}
		else if (m_iter != m_track->getClips().end() && m_iter.key() < nextpos)
		{
			int f;
			if (m_iter.key() < playpos) // started before the current play pos
			{
				if (m_iter.value()->getEndTime() <= playpos) // ended before the current play pos, so skip it
				{
					++ m_iter;
					continue;
				}
				else // ends after the current play pos, so start it playing asap
					f = firstframe;
			}
			else // starts after the current play pos, so start it playing after a while
				f = firstframe + (int)floor( (m_iter.key() - playpos) / m_sp->m_beatsPerSample );

			m_playingClip = m_iter.value();

			playpos += (f - firstframe) * m_sp->m_beatsPerSample;

			StreamEvent spe;
			spe.type = StreamEventType::patternPlay;
			spe.time = f;
			spe.pattern.track = m_track;
			spe.pattern.hostPattern = m_playingClip->m_pattern;
			spe.pattern.pos = m_playingClip->m_begin + (playpos - m_iter.key());
			m_events.insert(spe);

			firstframe = f;
			++ m_iter;
		}
		else
		{
			break;
		}
	}
}
