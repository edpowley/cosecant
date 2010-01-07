#pragma once

#include "machine.h"

namespace Seq
{
	class Track; class Clip;

	class Sequence : public Object
	{
		Q_OBJECT

	public:
		int getNumTracks() { return m_tracks.length(); }
		Ptr<Track> getTrack(int index) { return m_tracks.value(index); }

		void insertTrack(int index, const Ptr<Track>& track);
		void removeTrack(int index);

		QMap<int, Ptr<Track> > getTracksForMachine(const Ptr<Machine>& mac);

	signals:
		void signalAddTrack(int index, const Ptr<Seq::Track>& track);
		void signalRemoveTrack(int index, const Ptr<Seq::Track>& track);

	protected:
		QList< Ptr<Track> > m_tracks;
	};

	class Track : public Object
	{
		Q_OBJECT

	public:
		Track(const Ptr<Machine>& mac) : m_mac(mac) {}

		const Ptr<Machine>& getMachine() { return m_mac; }

	protected:
		QMap<int64, Ptr<Clip> > m_clips;
		Ptr<Machine> m_mac;
	};

	class Clip : public Object
	{
	protected:
		int64 m_length;
	};

};

using Seq::Sequence;
