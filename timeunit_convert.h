#pragma once

#include "cosecant_api.h"

double ConvertTimeUnits(CosecantAPI::TimeUnit::unit from,
						CosecantAPI::TimeUnit::unit to,
						double value,
						const CosecantAPI::TimeInfo* timeinfo = NULL );

inline double ConvertTimeUnits(const CosecantAPI::TimeValue& from,
							   CosecantAPI::TimeUnit::unit to,
							   const CosecantAPI::TimeInfo* timeinfo = NULL )
{
	return ConvertTimeUnits(from.unit, to, from.value, timeinfo);
}
