#pragma once

#include "cosecant_api.h"
using namespace CosecantAPI;

#include "sequenceevent.h"

namespace WorkBuffer { class Base; class ParamControl; class SequenceEvents; };

namespace DelayLine
{
	class Base : public ObjectWithUuid
	{
	public:
		Base(int length) : m_length(length), m_readHead(0), m_writeHead(0) {}

		int getLength() { return m_length; }
		void setLength(int length) { m_length = length; reallocate(); }

		void setReadHead(int h)  { m_readHead  = truemod(h, m_length); }
		void setWriteHead(int h) { m_writeHead = truemod(h, m_length); }
		int getReadHead()  { return m_readHead;  }
		int getWriteHead() { return m_writeHead; }

		virtual void read (WorkBuffer::Base* buf, int firstframe, int lastframe) = 0;
		virtual void write(WorkBuffer::Base* buf, int firstframe, int lastframe) = 0;

		inline void advance(int& head, int step) { head = (head + step) % m_length; }

	protected:
		virtual void reallocate() = 0;

		int m_length;
		int m_readHead, m_writeHead;
	};

	class Audio : public Base
	{
	public:
		Audio(int length, unsigned int nChannels);
		const unsigned int m_nChannels;
		std::vector<float> m_data;

		virtual void read (WorkBuffer::Base* buf, int firstframe, int lastframe);
		virtual void write(WorkBuffer::Base* buf, int firstframe, int lastframe);

	protected:
		virtual void reallocate();
	};

	template<class TBuffer, class TMap> class EventMap : public Base
	{
	public:
		EventMap(int length) : Base(length) {}
		TMap m_data;

		virtual void read (WorkBuffer::Base* buf, int firstframe, int lastframe);
		virtual void write(WorkBuffer::Base* buf, int firstframe, int lastframe);

	protected:
		virtual void reallocate() { m_data.clear(); }
	};

	class ParamControl : public EventMap< WorkBuffer::ParamControl, std::map<int, ParamValue> >
	{
	public:
		ParamControl(int length) : EventMap(length) {}
	};

	class SequenceEvents : public EventMap< WorkBuffer::SequenceEvents, std::multimap<int, Ptr<SequenceEvent::Base> > >
	{
	public:
		SequenceEvents(int length) : EventMap(length) {}
	};
};
