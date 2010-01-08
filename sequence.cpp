#include "stdafx.h"
#include "common.h"

#include "sequence.h"
using namespace Seq;

Sequence::Sequence()
{
	m_tempoClips.insert(0, new TempoClip::Fixed(126));
}

void Sequence::insertTrack(int index, const Ptr<Track>& track)
{
	m_tracks.insert(index, track);
	signalAddTrack(index, track);
}

void Sequence::removeTrack(int index)
{
	Ptr<Track> track = getTrack(index);
	m_tracks.removeAt(index);
	signalRemoveTrack(index, track);
}

QMap<int, Ptr<Track> > Sequence::getTracksForMachine(const Ptr<Machine>& mac)
{
	QMap<int, Ptr<Track> > ret;
	
	for (int i=0; i<m_tracks.length(); i++)
	{
		if (getTrack(i)->getMachine() == mac)
			ret.insert(i, m_tracks[i]);
	}

	return ret;
}

int64 Sequence::getSnapPoint(int64 x)
{
	QMap<int64, Ptr<TempoClip::Base> >::const_iterator iter = m_tempoClips.upperBound(x);
	// Now iter points to the first clip with key > x

	if (iter != m_tempoClips.begin()) --iter;
	// Now iter points to the last clip with key <= x

	return iter.key() + iter.value()->getSnapPoint(x - iter.key());
}

///////////////////////////////////////////////////////////////////////

TempoClip::Fixed::Fixed(double bpm)
{
	setBpm(bpm);
}

void TempoClip::Fixed::setBpm(double bpm)
{
	m_bpm = bpm;
	m_pulsesPerBeat = (int64)(60.0 / bpm * (double)(1LL << 32));
}

int64 TempoClip::Fixed::getSnapPoint(int64 x)
{
	return (int64)( floor((double)x / (double)m_pulsesPerBeat + 0.5) * m_pulsesPerBeat );
}
