#pragma once

#include "cosecant_api.h"

double ConvertTimeUnits(CosecantAPI::TimeUnit::unit from, CosecantAPI::TimeUnit::unit to, double value);

inline double ConvertTimeUnits(const CosecantAPI::TimeValue& from, CosecantAPI::TimeUnit::unit to)
{ return ConvertTimeUnits(from.unit, to, from.value); }
