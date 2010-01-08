#pragma once

#include "machine.h"

namespace Seq
{
	namespace TempoClip
	{
		class Base : public Object
		{
		public:
			virtual int64 getSnapPoint(int64 x) = 0;
		};

		class Fixed : public Base
		{
		public:
			Fixed(double bpm);

			int64 getSnapPoint(int64 x);

		protected:
			int64 m_pulsesPerBeat;
			double m_bpm;

			void setBpm(double bpm);
		};
	};

	///////////////////////////////////////////////////////////////////////////

	class Clip : public Object
	{
	protected:
		int64 m_length;
	};

	///////////////////////////////////////////////////////////////////////////

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

	///////////////////////////////////////////////////////////////////////////

	class Sequence : public Object
	{
		Q_OBJECT

	public:
		Sequence();

		int getNumTracks() { return m_tracks.length(); }
		Ptr<Track> getTrack(int index) { return m_tracks.value(index); }
		int getTrackIndex(const Ptr<Track>& track) { return m_tracks.indexOf(track); }

		void insertTrack(int index, const Ptr<Track>& track);
		void removeTrack(int index);

		QMap<int, Ptr<Track> > getTracksForMachine(const Ptr<Machine>& mac);

		int64 getSnapPoint(int64 x);

	signals:
		void signalAddTrack(int index, const Ptr<Seq::Track>& track);
		void signalRemoveTrack(int index, const Ptr<Seq::Track>& track);

	protected:
		QList< Ptr<Track> > m_tracks;

		QMap<int64, Ptr<TempoClip::Base> > m_tempoClips;
	};

};

using Seq::Sequence;
