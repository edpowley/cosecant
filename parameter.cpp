#include "stdafx.h"
#include "common.h"
#include "parameter.h"
#include "timeunit_convert.h"

void Parameter::Base::setParamPin(const Ptr<ParamPin>& pin)
{
	if (m_paramPin) signalRemoveParamPin();
	m_paramPin = pin;
	if (m_paramPin) signalAddParamPin();
}

Parameter::Group::Group(const Ptr<Machine>& mac, const ParamGroupInfo* info)
: Base(mac)
{
	setName(info->p.name);

	if (info->params)
	{
		for (const ParamInfo** p = info->params; *p; ++p)
		{
			Ptr<Parameter::Base> newpar;

			switch ((*p)->type)
			{
			case ParamType::tGroup:
				newpar = new Group(mac, reinterpret_cast<const ParamGroupInfo*>(*p) );
				break;
			case ParamType::tReal:
				newpar = new Real(mac, reinterpret_cast<const RealParamInfo*>(*p) );
				break;
			case ParamType::tInt:
				newpar = new Int(mac, reinterpret_cast<const IntParamInfo*>(*p) );
				break;
			case ParamType::tTime:
				newpar = new Time(mac, reinterpret_cast<const TimeParamInfo*>(*p) );
				break;
			case ParamType::tEnum:
				newpar = new Enum(mac, reinterpret_cast<const EnumParamInfo*>(*p) );
				break;
			}

			ASSERT(newpar != NULL);
			m_params.push_back(newpar);
		}
	}
}

void Parameter::Group::initParamStuff(Machine* mac)
{
	BOOST_FOREACH(Ptr<Parameter::Base> p, m_params)
	{
		p->initParamStuff(mac);
	}
}

Parameter::Scalar::Scalar(const Ptr<Machine>& mac, ParamTag tag)
: Base(mac), m_tag(tag), m_min(0), m_max(1), m_def(0), m_state(0), m_scale(ParamScale::linear), m_freeRange(false)
{
}

void Parameter::Scalar::initParamStuff(Machine* mac)
{
	mac->m_paramMap[m_tag] = this;
	mac->m_paramChanges[m_tag] = m_state;
}

void Parameter::Scalar::setRange(double min, double max, ParamFlags::i flags)
{
	if (flags & ParamFlags::noMin)
	{
		m_min = -DBL_MAX;
		m_freeRange = true;
	}
	else
	{
		m_min = min;
	}

	if (flags & ParamFlags::noMax)
	{
		m_max = DBL_MAX;
		m_freeRange = true;
	}
	else
	{
		m_max = max;
	}
}

void Parameter::Scalar::setDefault(double def)
{
	m_def = sanitise(def);
}

void Parameter::Scalar::setState(double state)
{
	m_state = sanitise(state);
}

void Parameter::Scalar::setScale(ParamScale::e scale)
{
	m_scale = scale;
}

void Parameter::Scalar::change(double newval)
{
	newval = sanitise(newval);
	if (newval != m_state)
	{
		setState(newval);
		valueChanged(newval);

		QMutexLocker lock(&m_mac->m_paramChangesMutex);
		m_mac->m_paramChanges[m_tag] = newval;
	}
}

Parameter::Real::Real(const Ptr<Machine>& mac, const RealParamInfo* info)
: Scalar(mac, info->p.tag)
{
	setName(info->p.name);
	setScale(static_cast<ParamScale::e>(info->scale));
	setRange(info->min, info->max, info->p.flags);
	setDefault(info->def);
	setState(info->def);
}

double Parameter::Real::sanitise(double v)
{
	return clamp(v, m_min, m_max);
}

Parameter::Int::Int(const Ptr<Machine>& mac, const IntParamInfo* info)
: Scalar(mac, info->p.tag)
{
	setName(info->p.name);
	setRange(info->min, info->max, info->p.flags);
	setDefault(info->def);
	setState(info->def);
}

double Parameter::Int::sanitise(double v)
{
	return clamp(floor(v+0.5), m_min, m_max);
}

Parameter::Time::Time(const Ptr<Machine>& mac, const TimeParamInfo* info)
:	Scalar(mac, info->p.tag),
	m_tmin(info->min), m_tmax(info->max), m_tdef(info->def), m_tstate(info->def),
	m_internalUnit(static_cast<TimeUnit::e>(info->internalUnit)),
	m_displayUnit(static_cast<TimeUnit::e>(info->defaultDisplayUnit)),
	m_displayUnits(info->displayUnits)
{
	setName(info->p.name);
	setRange(ConvertTimeUnits(m_tmin, m_internalUnit), ConvertTimeUnits(m_tmax, m_internalUnit), info->p.flags);
	setDefault(ConvertTimeUnits(m_tdef, m_internalUnit));
	setState(getDefault());
}

double Parameter::Time::sanitise(double v)
{
	return v;
}

Parameter::Enum::Enum(const Ptr<Machine>& mac, const EnumParamInfo* info)
:	Scalar(mac, info->p.tag)
{
	setName(info->p.name);

	if (info->items)
	{
		QStringList items;
		for (const char** item = info->items; *item; ++item)
		{
			items << *item;
		}
		setItems(items);
	}

	setDefault(info->def);
	setState(info->def);
}

double Parameter::Enum::sanitise(double v)
{
	return clamp(floor(v+0.5), m_min, m_max);
}

void Parameter::Enum::setItems(const QStringList& items)
{
	m_items = items;
	setRange(0, m_items.length()-1, 0);
}
