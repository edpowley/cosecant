#include "stdafx.h"
#include "common.h"

#include "sequence.h"
using namespace Seq;

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
