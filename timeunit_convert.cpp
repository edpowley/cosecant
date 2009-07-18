#include "stdafx.h"
#include "common.h"
#include "timeunit_convert.h"
using namespace CosecantAPI;
using namespace CosecantAPI::TimeUnit;
#include "seqplay.h"

double fromSamples(unit u, double x, const TimeInfo& ti)
{
	switch (u)
	{
	case seconds:	return x / ti.samplesPerSecond;
	case beats:		return x / ti.samplesPerSecond * ti.beatsPerSecond;
	case samples:	return x;
	case hertz:		return ti.samplesPerSecond / x;
	case fracfreq:	return 1.0 / x;
	case notenum:	return 69.0 + 12.0 * log(fromSamples(hertz, x, ti) / 440.0) / log(2.0);
	default:		return x;
	}
}

double toSamples(TimeUnit::unit u, double x, const TimeInfo& ti)
{
	switch (u)
	{
	case seconds:	return x * ti.samplesPerSecond;
	case beats:		return x * ti.samplesPerSecond / ti.beatsPerSecond;
	case samples:	return x;
	case hertz:		return ti.samplesPerSecond / x;
	case fracfreq:	return 1.0 / x;
	case notenum:	return toSamples(hertz, 440.0 * pow(2.0, (x - 69.0) / 12.0), ti);
	default:		return x;
	}
}

double ConvertTimeUnits(TimeUnit::unit from, TimeUnit::unit to, double value, const TimeInfo* timeinfo)
{
	if (from == to) return value;

	if (!timeinfo) timeinfo = &SeqPlay::get().getTimeInfo();

	return fromSamples(	to,
						toSamples(from, value, *timeinfo),
						*timeinfo );
}
