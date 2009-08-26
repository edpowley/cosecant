#include "stdafx.h"
#include "common.h"
#include "eventstream.h"

// Return the index of an element with the specified time
// If no such element exists, return the index of the last element with time less than specified
int EventStream::binarySearch(int time) const
{
	int a = 0;
	int b = m_list.length() - 1;

	if (m_list[a]->time == time) return a;
	if (m_list[a]->time >  time) return a-1;
	if (m_list[b]->time == time) return b;
	if (m_list[b]->time <  time) return b;

	// Invariant: m_list[a]->time <= time <= m_list[b]->time

	while (a != b)
	{
		if (b == a+1)
		{
			// Can't have at({a,b}).time == time or we would've noticed it
			// return the lower one
			return a;
		}
		else
		{
			int c = (a+b)/2;
			int ctime = m_list[c]->time;
			if (time < ctime)
				b = c;
			else if (time > ctime)
				a = c;
			else // time == ctime
				return c;
		}
	}

	// now a == b
	return a;
}

int EventStream::lowerBoundIndex(int time) const
{
	if (empty()) return -1;

	int i = binarySearch(time);
	// now i is the index of an element with at(i).time == time, or the last with at(i).time < time

	while (i >= 0 && m_list[i]->time == time) --i;
	if (i < 0) return 0;
	// now i is the index of the last element with at(i).time < time

	++i;
	// now i is the index of the first element with at(i).time >= time

	return i;
}

int EventStream::upperBoundIndex(int time) const
{
	if (empty()) return -1;

	int i = binarySearch(time);
	// now i is the index of an element with at(i).time == time, or the last with at(i).time < time

	if (i < 0) return 0;

	while (i < m_list.length() && m_list[i]->time == time) ++i;
	if (i >= m_list.length()) return -1;
	// now i is the index of the last element with at(i).time <= time

	++i;
	// now i is the index of the first element with at(i).time > time

	return i;
}

int EventStream::findIndex(int time) const
{
	if (empty()) return -1;

	int i = binarySearch(time);
	// now i is the index of an element with at(i).time == time, or the last with at(i).time < time

	// If not found...
	if (m_list[i]->time != time) return -1;

	while (i >= 0 && m_list[i]->time == time) --i;
	// now i is the index of the last element with at(i).time < time

	++i;
	// now i is the index of the first element with at(i).time == time

	return i;
}

////////////////////////////////////////////////////////////////////////////////////

EventStream::iterator EventStream::insert(const CosecantAPI::StreamEvent& ev)
{
	iterator pos = upperBound(ev.time);
	return m_list.insert(pos, new Event(ev));
}

EventStream::iterator EventStream::erase(iterator iter)
{
	delete iter.ptr();
	return m_list.erase(iter);
}

void EventStream::clear()
{
	iterator iter = begin();
	while (iter != end())
		iter = erase(iter);
}

void EventStream::offsetTimes(int offset)
{
	for(iterator iter = begin(); iter != end(); ++iter)
	{
		Event* ev = const_cast<Event*>(iter.ptr());
		ev->time += offset;
	}
}
