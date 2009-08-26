#include "stdafx.h"
#include "common.h"
#include "delayline.h"
using namespace DelayLine;
#include "workbuffer.h"

Audio::Audio(int length, unsigned int nChannels)
: Circular(length), m_nChannels(nChannels)
{
	reallocate();
}

void Audio::reallocate()
{
	m_data.resize(m_length * m_nChannels, 0.0f);
}

void Audio::read(WorkBuffer::Base* bufb, int firstframe, int lastframe)
{
	WorkBuffer::Audio* buf = dynamic_cast<WorkBuffer::Audio*>(bufb);
	const unsigned int nc = m_nChannels;
	ASSERT(buf->m_nChannels == nc);
	for (int f=firstframe; f<lastframe; f++)
	{
		for (unsigned int c=0; c<nc; c++)
		{
			buf->m_data[f * nc + c] = m_data[m_readHead * nc + c];
		}

		advance(m_readHead, 1);
	}
}

void Audio::write(WorkBuffer::Base* bufb, int firstframe, int lastframe)
{
	WorkBuffer::Audio* buf = dynamic_cast<WorkBuffer::Audio*>(bufb);
	const unsigned int nc = m_nChannels;
	ASSERT(buf->m_nChannels == nc);
	for (int f=firstframe; f<lastframe; f++)
	{
		for (unsigned int c=0; c<nc; c++)
		{
			m_data[m_writeHead * nc + c] = buf->m_data[f * nc + c];
		}

		advance(m_writeHead, 1);
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void ParamControl::read(WorkBuffer::Base* bufb, int firstframe, int lastframe)
{
	WorkBuffer::ParamControl* buf = dynamic_cast<WorkBuffer::ParamControl*>(bufb);

	Data::iterator iter = m_data.begin();
	
	// Consume some elements
	while (iter != m_data.end() && iter->first < lastframe-firstframe)
	{
		buf->m_data.insert(std::make_pair(iter->first + firstframe, iter->second));
	}

	// Adjust timestamps of the rest
	for (; iter != m_data.end(); ++iter)
	{
		iter->first -= lastframe-firstframe;
	}
}

void ParamControl::write(WorkBuffer::Base* bufb, int firstframe, int lastframe)
{
	WorkBuffer::ParamControl* buf = dynamic_cast<WorkBuffer::ParamControl*>(bufb);

	for (std::map<int,double>::const_iterator iter = buf->m_data.begin(); iter != buf->m_data.end(); ++iter)
	{
		m_data.append(qMakePair(iter->first - firstframe + m_length, iter->second));
	}
}

//////////////////////////////////////////////////////////////////////////////////////

void Events::read(WorkBuffer::Base* bufb, int firstframe, int lastframe)
{
	WorkBuffer::Events* buf = dynamic_cast<WorkBuffer::Events*>(bufb);

	EventStream::iterator iter = m_data.begin();
	
	// Consume some elements
	while (iter != m_data.end() && iter->time < lastframe-firstframe)
	{
		StreamEvent ev = *iter;
		ev.time += firstframe;
		buf->m_data.insert(ev);
		iter = m_data.erase(iter);
	}

	// Adjust timestamps of the rest
	m_data.offsetTimes(-(lastframe-firstframe));
}

void Events::write(WorkBuffer::Base* bufb, int firstframe, int lastframe)
{
	WorkBuffer::Events* buf = dynamic_cast<WorkBuffer::Events*>(bufb);

	for (EventStream::iterator iter = buf->m_data.begin(); iter != buf->m_data.end(); ++iter)
	{
		StreamEvent ev = *iter;
		ev.time = ev.time - firstframe + m_length;
		m_data.insert(ev);
	}
}
