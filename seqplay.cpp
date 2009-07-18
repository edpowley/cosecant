#include "stdafx.h"
#include "common.h"
#include "seqplay.h"

SingletonPtr<SeqPlay> SeqPlay::s_singleton;

SeqPlay::SeqPlay()
{
	m_timeinfo.beatsPerSecond = 120.0 / 60.0;
	m_timeinfo.beatsPerBar = 4;
	m_timeinfo.beatsPerWholeNote = 4;
	m_timeinfo.barsPerSmallGrid = 4;
	m_timeinfo.smallGridsPerLargeGrid = 4;
	m_timeinfo.samplesPerSecond = 44100;
}
