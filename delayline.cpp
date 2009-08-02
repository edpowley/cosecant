#include "stdafx.h"
#include "common.h"
#include "delayline.h"
using namespace DelayLine;
#include "workbuffer.h"

Audio::Audio(int length, unsigned int nChannels)
: Base(length), m_nChannels(nChannels)
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

template<class TBuffer, class TMap>
void EventMap<TBuffer, TMap>::read(WorkBuffer::Base* bufb, int firstframe, int lastframe)
{
	if (m_readHead + (lastframe - firstframe) > m_length)
	{
		int middleframe = firstframe + (m_length - m_readHead);
		read(bufb, firstframe, middleframe);
		read(bufb, middleframe, lastframe);
	}
	else
	{
		TBuffer* buf = dynamic_cast<TBuffer*>(bufb);
		TMap::const_iterator begin = m_data.lower_bound(m_readHead);
		TMap::const_iterator end   = m_data.lower_bound(m_readHead + (lastframe - firstframe));
		for (TMap::const_iterator iter = begin; iter != end; ++iter)
		{
			buf->m_data.insert(std::make_pair(iter->first - m_readHead + firstframe, iter->second));
		}

		advance(m_readHead, lastframe - firstframe);
	}
}

template<class TBuffer, class TMap>
void EventMap<TBuffer, TMap>::write(WorkBuffer::Base* bufb, int firstframe, int lastframe)
{
	if (m_writeHead + (lastframe - firstframe) > m_length)
	{
		int middleframe = firstframe + (m_length - m_writeHead);
		write(bufb, firstframe, middleframe);
		write(bufb, middleframe, lastframe);
	}
	else
	{
		TBuffer* buf = dynamic_cast<TBuffer*>(bufb);

		m_data.erase(
			m_data.lower_bound(m_writeHead),
			m_data.lower_bound(m_writeHead + (lastframe - firstframe)));

		TMap::const_iterator begin = buf->m_data.lower_bound(firstframe);
		TMap::const_iterator end   = buf->m_data.lower_bound(lastframe);

		for (TMap::const_iterator iter = begin; iter != end; ++iter)
		{
			m_data.insert(std::make_pair(iter->first - firstframe + m_writeHead, iter->second));
		}

		advance(m_writeHead, lastframe - firstframe);
	}
}

// Instantiations, so that the linker can find these functions
#define EVENTMAP_INSTANTIATE(...)												\
	template void EventMap< __VA_ARGS__ >::read(WorkBuffer::Base*, int, int);	\
	template void EventMap< __VA_ARGS__ >::write(WorkBuffer::Base*, int, int);	\
// end define

EVENTMAP_INSTANTIATE(WorkBuffer::ParamControl, std::map<int, double>)
EVENTMAP_INSTANTIATE(WorkBuffer::SequenceEvents, WorkBuffer::SequenceEvents::EventMap)
