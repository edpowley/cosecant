#include "stdafx.h"
#include "common.h"
#include "seqplay.h"
#include "song.h"
#include "sequence.h"

SingletonPtr<SeqPlay> SeqPlay::s_singleton;

SeqPlay::SeqPlay()
: m_playing(false), m_playPos(0)
{
	m_timeinfo.beatsPerSecond = 120.0 / 60.0;
	m_timeinfo.beatsPerBar = 4;
	m_timeinfo.beatsPerWholeNote = 4;
	m_timeinfo.barsPerSmallGrid = 4;
	m_timeinfo.smallGridsPerLargeGrid = 4;
	m_timeinfo.samplesPerSecond = 44100;
}

void SeqPlay::setPlaying(bool playing)
{
	m_playing = playing;
}

void SeqPlay::work(int firstframe, int lastframe)
{
	if (!m_playing) return;
	if (firstframe >= lastframe) return;

	Ptr<Sequence::Seq> seq = Song::get().m_sequence;
	QReadLocker(&seq->m_mutex);

	double beatsPerSample = m_timeinfo.beatsPerSecond / m_timeinfo.samplesPerSecond;
	double nextpos = m_playPos + (lastframe - firstframe) * beatsPerSample;
	
	// Check for looping
	if (nextpos >= seq->m_loopEnd)
	{
		// Work the bit before the loop, then the bit after the loop
		int loopframe = firstframe + (int)floor( (seq->m_loopEnd - m_playPos) / beatsPerSample );
		work(firstframe, loopframe);
		m_playPos -= (seq->m_loopEnd - seq->m_loopStart);
		work(loopframe, lastframe);
		return;
	}

	m_playPos = nextpos;
}
