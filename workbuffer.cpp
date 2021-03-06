#include "stdafx.h"
#include "common.h"
#include "workbuffer.h"
using namespace WorkBuffer;

std::map<SignalType::e, Ptr<Factory_Base> > WorkBuffer::s_factories;
FactoriesInitialiser facinit;
FactoriesInitialiser::FactoriesInitialiser()
{
	s_factories[SignalType::monoAudio]		= new Factory<MonoAudio>("monoAudio");
	s_factories[SignalType::stereoAudio]	= new Factory<StereoAudio>("stereoAudio");
	s_factories[SignalType::paramControl]	= new Factory<ParamControl>("paramControl");
	s_factories[SignalType::noteTrigger]	= new Factory<Events>("noteTrigger");
}

////////////////////////////////////////////////////////////////////////////

Audio::Audio(unsigned int nChannels) : m_nChannels(nChannels)
{
	m_data.resize(AudioIO::c_maxBufferSize * m_nChannels);
}

void Audio::clear(int firstframe, int lastframe)
{
	for (unsigned int i = firstframe * m_nChannels; i<lastframe * m_nChannels; i++)
		m_data[i] = 0.0f;
}

void Audio::clearAll()
{
	clear(0, AudioIO::c_maxBufferSize);
}

void Audio::copy(Base* other, int firstframe, int lastframe)
{
	Audio* p = dynamic_cast<Audio*>(other);
	if (p)
	{
		for (unsigned int i = firstframe * m_nChannels; i<lastframe * m_nChannels; i++)
			m_data[i] = p->m_data[i];
	}
}

void Audio::mix(Base* other, int firstframe, int lastframe)
{
	Audio* p = dynamic_cast<Audio*>(other);
	if (p)
	{
		for (unsigned int i = firstframe * m_nChannels; i<lastframe * m_nChannels; i++)
			m_data[i] += p->m_data[i];
	}
}

////////////////////////////////////////////////////////////////////////////////////////

void ParamControl::clear(int firstframe, int lastframe)
{
	m_data.erase(m_data.lower_bound(firstframe), m_data.lower_bound(lastframe));
}

void ParamControl::clearAll()
{
	m_data.clear();
}

void ParamControl::preProcess()
{
	if (!m_data.empty())
		m_lastValue = m_data.rbegin()->second;
	m_data.clear();
}

void ParamControl::copy(Base* other, int firstframe, int lastframe)
{
	if (ParamControl* p = dynamic_cast<ParamControl*>(other))
	{
		clear(firstframe, lastframe);
		m_data.insert(p->m_data.lower_bound(firstframe), p->m_data.lower_bound(lastframe));
		if (p->m_data.lower_bound(firstframe) == p->m_data.begin())
			m_lastValue = p->m_lastValue;
	}
}

void ParamControl::mix(Base* other, int firstframe, int lastframe)
{
	if (ParamControl* p = dynamic_cast<ParamControl*>(other))
	{
		std::map<int, double> newdata;

		std::map<int, double>::const_iterator
			iterThis	=    m_data.lower_bound(firstframe),
			iterOther	= p->m_data.lower_bound(firstframe),
			endThis		=    m_data.lower_bound(lastframe),
			endOther	= p->m_data.lower_bound(lastframe);

		double lastThis, lastOther;

		if (iterThis != m_data.begin())
		{
			std::map<int, double>::const_iterator i = iterThis; --i; lastThis = i->second;
		}
		else
		{
			lastThis = m_lastValue;
		}

		if (iterOther != p->m_data.begin())
		{
			std::map<int, double>::const_iterator i = iterOther; --i; lastOther = i->second;
		}
		else
		{
			lastOther = p->m_lastValue;
		}

		while (iterThis != endThis && iterOther != endOther)
		{
			const int somethingBig = 0x80000000;
			int timeThis  = (iterThis  != endThis ) ? iterThis ->first : somethingBig;
			int timeOther = (iterOther != endOther) ? iterOther->first : somethingBig;

			// NB: if timeThis == timeOther, both of these "if"s are executed. That's intentional.

			if (timeThis <= timeOther)
			{
				lastThis = iterThis->second;
				++ iterThis;
			}

			if (timeOther <= timeThis)
			{
				lastOther = iterOther->second;
				++ iterOther;
			}

			newdata[min(timeThis, timeOther)] = lastThis + lastOther;
		}

		m_lastValue += p->m_lastValue;

		std::swap(m_data, newdata);
	}
}

void ParamControl::getEventBreakPoints(std::set<int>& bp)
{
	for (std::map<int,double>::const_iterator iter = m_data.begin(); iter != m_data.end(); ++iter)
	{
		bp.insert(iter->first);
	}
}

////////////////////////////////////////////////////////////////////////////////////////

void Events::clear(int firstframe, int lastframe)
{
	EventStream::iterator iter = m_data.lowerBound(firstframe);
	EventStream::iterator enditer = m_data.upperBound(lastframe-1);
	while (iter != enditer)
		iter = m_data.erase(iter);
}

void Events::clearAll()
{
	m_data.clear();
}

void Events::copy(Base* other, int firstframe, int lastframe)
{
	if (Events* p = dynamic_cast<Events*>(other))
	{
		clear(firstframe, lastframe);
		mix(p, firstframe, lastframe);
	}
}

void Events::mix(Base* other, int firstframe, int lastframe)
{
	if (Events* p = dynamic_cast<Events*>(other))
	{
		EventStream::const_iterator enditer = p->m_data.upperBound(lastframe-1);
		for (EventStream::const_iterator iter = p->m_data.lowerBound(firstframe);
			iter != enditer; ++iter)
		{
			m_data.insert(*iter);
		}
	}
}

void Events::getEventBreakPoints(std::set<int>& bp)
{
	for (EventStream::const_iterator iter = m_data.begin(); iter != m_data.end(); ++iter)
	{
		bp.insert(iter->time);
	}
}
