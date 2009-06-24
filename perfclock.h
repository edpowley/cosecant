#pragma once

class PerfClock
{
public:
	PerfClock() { reset(); }

	void reset()
	{
		QueryPerformanceCounter(&m_startTime);
	}

	__int64 getElapsed()
	{
		LARGE_INTEGER now; QueryPerformanceCounter(&now);
		return now.QuadPart - m_startTime.QuadPart;
	}

protected:
	LARGE_INTEGER m_startTime;
};

class PerfClockAutoCount : public PerfClock
{
public:
	PerfClockAutoCount(__int64* count, __int64* subtract = NULL) : m_count(count), m_subtract(subtract), PerfClock() {}
	~PerfClockAutoCount()
	{
		__int64 elapsed = getElapsed();
		*m_count += elapsed;
		if (m_subtract) *m_subtract -= elapsed;
	}

protected:
	__int64* m_count;
	__int64* m_subtract;

	using PerfClock::reset;
	using PerfClock::getElapsed;
};
