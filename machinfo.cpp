#include "stdafx.h"
#include "common.h"
#include "machinfo.h"

ParamInfo::Scalar::Scalar(const QString& name, ParamTag tag, double mn, double mx, double def, unsigned long flags)
: Base(name, tag), m_min(mn), m_max(mx), m_def(clamp(def, mn, mx))
{
	m_isLogarithmic = flags & ParamFlags::logarithmic;
	m_isInteger = flags & ParamFlags::integer;

	if (m_isLogarithmic && (m_min <= 0.0 || m_max <= 0.0))
		throw Error(QString(
			"Error creating parameter '%1': logarithmic parameters must have min > 0 && max > 0.")
			.arg(m_name));
}


ParamInfo::Time::Time(const QString& name, ParamTag tag, 
					  TimeUnit::unit internalUnit, double mn, double mx, double def,
					  unsigned int guiUnits, TimeUnit::unit guiDefaultUnit)
: Scalar(name, tag, mn, mx, def, 0), m_internalUnit(internalUnit), m_guiUnits(guiUnits), m_guiUnit(guiDefaultUnit)
{
	if ((guiUnits & guiDefaultUnit) == 0)
		throw Error(QString(
			"Error creating parameter '%1': guiDefaultUnit is not in guiUnits.")
			.arg(m_name));
}

ParamInfo::Enum::Enum(const QString& name, ParamTag tag, const std::vector<QString>& items, double def)
: Scalar(name, tag, 0, items.size()-1, def, ParamFlags::integer), m_items(items)
{
}

void ParamInfo::Enum::itemsChanged()
{
	m_min = 0;
	m_max = m_items.size() - 1;
	m_def = clamp(m_def, m_min, m_max);
	signalRangeChange();
}
