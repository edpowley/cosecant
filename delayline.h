#pragma once

#include "cosecant_api.h"
using namespace CosecantAPI;

//namespace WorkBuffer { class Base; class ParamControl; class EventStream; };

namespace DelayLine
{
	class Base : public ObjectWithUuid
	{
	public:
		Base(int length) : m_length(length) {}

		int getLength() { return m_length; }
		void setLength(int length) { m_length = length; reallocate(); }

		virtual void read (WorkBuffer::Base* buf, int firstframe, int lastframe) = 0;
		virtual void write(WorkBuffer::Base* buf, int firstframe, int lastframe) = 0;

	protected:
		virtual void reallocate() = 0;
		int m_length;
	};

	class Circular : public Base
	{
	public:
		Circular(int length) : Base(length), m_readHead(0), m_writeHead(0) {}

	protected:
		int m_readHead, m_writeHead;

		void setReadHead(int h)  { m_readHead  = truemod(h, m_length); }
		void setWriteHead(int h) { m_writeHead = truemod(h, m_length); }
		int getReadHead()  { return m_readHead;  }
		int getWriteHead() { return m_writeHead; }

		inline void advance(int& head, int step) { head = (head + step) % m_length; }
	};

	class Audio : public Circular
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

	class ParamControl : public Base
	{
	public:
		ParamControl(int length) : Base(length) {}
		typedef QList< QPair<int, double> > Data;
		Data m_data;

		virtual void read (WorkBuffer::Base* buf, int firstframe, int lastframe);
		virtual void write(WorkBuffer::Base* buf, int firstframe, int lastframe);

	protected:
		virtual void reallocate() { m_data.clear(); }
	};

	class EventStream : public Base
	{
	public:
		EventStream(int length) : Base(length) {}
		typedef QList< QPair<int, StreamEvent> > Data;
		Data m_data;

		virtual void read (WorkBuffer::Base* buf, int firstframe, int lastframe);
		virtual void write(WorkBuffer::Base* buf, int firstframe, int lastframe);

	protected:
		virtual void reallocate() { m_data.clear(); }
	};
};
