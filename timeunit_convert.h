#pragma once

#include "cosecant_api.h"

double ConvertTimeUnits(CosecantAPI::TimeUnit::e from,
						CosecantAPI::TimeUnit::e to,
						double value,
						const CosecantAPI::TimeInfo* timeinfo = NULL );

inline double ConvertTimeUnits(const CosecantAPI::TimeValue& from,
							   CosecantAPI::TimeUnit::e to,
							   const CosecantAPI::TimeInfo* timeinfo = NULL )
{
	return ConvertTimeUnits(static_cast<CosecantAPI::TimeUnit::e>(from.unit), to, from.value, timeinfo);
}
