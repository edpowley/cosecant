#pragma once

#include "cosecant_api.h"
#include "sequence.h"

namespace SequenceEvent
{
	class Base : public Object
	{
	public:
		virtual void work(const Ptr<Machine>& mac) = 0;
	};

	class Note : public Base
	{
	public:
		Note(CosecantAPI::NoteEvent data) : m_data(data) {}
		CosecantAPI::NoteEvent m_data;

		virtual void work(const Ptr<Machine>& mac);
	};

	class Start : public Base
	{
	public:
		Start(const Ptr<Sequence::Track>& track, const Ptr<Sequence::Event>& ev, double pos)
			: m_track(track), m_ev(ev), m_pos(pos) {}
		
		virtual void work(const Ptr<Machine>& mac);

	protected:
		Ptr<Sequence::Track> m_track;
		Ptr<Sequence::Event> m_ev;
		double m_pos;
	};

	class Stop : public Base
	{
	public:
		Stop(const Ptr<Sequence::Track>& track, const Ptr<Sequence::Event>& ev)
			: m_track(track), m_ev(ev) {}

		virtual void work(const Ptr<Machine>& mac);

	protected:
		Ptr<Sequence::Track> m_track;
		Ptr<Sequence::Event> m_ev;
	};
};
