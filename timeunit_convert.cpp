#include "stdafx.h"
#include "common.h"
#include "timeunit_convert.h"
using namespace CosecantAPI;

double fromSamples(TimeUnit::unit u, double x)
{
	switch (u)
	{
	case TimeUnit::seconds:		return x / 44100.0;
	case TimeUnit::ticks:		return x / 44100.0 * 2.0;
	case TimeUnit::samples:		return x;
	case TimeUnit::hertz:		return 44100.0 / x;
	case TimeUnit::fracfreq:	return 1.0 / x;
	case TimeUnit::notenum:		return 69.0 + 12.0 * log(fromSamples(TimeUnit::hertz, x) / 440.0) / log(2.0);
	default:					return x;
	}
}

double toSamples(TimeUnit::unit u, double x)
{
	switch (u)
	{
	case TimeUnit::seconds:		return x * 44100.0;
	case TimeUnit::ticks:		return x * 44100.0 / 2.0;
	case TimeUnit::samples:		return x;
	case TimeUnit::hertz:		return 44100.0 / x;
	case TimeUnit::fracfreq:	return 1.0 / x;
	case TimeUnit::notenum:		return toSamples(TimeUnit::hertz, 440.0 * pow(2.0, (x - 69.0) / 12.0));
	default:					return x;
	}
}

double ConvertTimeUnits(TimeUnit::unit from, TimeUnit::unit to, double value)
{
	return fromSamples(to, toSamples(from, value));
}
