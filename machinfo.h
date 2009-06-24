#pragma once
#include "common.h"

#include "object.h"

#include "cosecant_api.h"
using namespace CosecantAPI;

class PinInfo : public Object
{
public:
	QString m_name;
	SignalType::st m_type;

	PinInfo(const QString& name, SignalType::st type) : m_name(name), m_type(type) {}
};

////////////////////////////////////////////////////////////////////////////

namespace ParamInfo
{
	class Base : public Object
	{
		Q_OBJECT

	public:
		QString m_name;
		ParamTag m_tag;

		Base(const QString& name, ParamTag tag) : m_name(name), m_tag(tag) {}

		virtual double sanitise(double value) { return value; }

	signals:
		void signalRangeChange();
	};

	class Group : public Base
	{
	public:
		std::vector< Ptr<Base> > m_params;

		Group(const QString& name, ParamTag tag) : Base(name, tag) {}

		Group* add(const Ptr<Base>& param)
		{	m_params.push_back(param); return this;   }
	};

	class Scalar : public Base
	{
	public:
		double m_min, m_max, m_def;
		bool m_isLogarithmic, m_isInteger;

		Scalar(const QString& name, ParamTag tag, double mn, double mx, double def, unsigned long flags);

		virtual double sanitise(double value)
		{
			if (m_isInteger)
				return clamp(ceil(value - 0.5), m_min, m_max);
			else
				return clamp(value, m_min, m_max);
		}
	};

	class Real : public Scalar
	{
	public:
		Real(const QString& name, ParamTag tag, double mn, double mx, double def, unsigned long flags)
			: Scalar(name, tag, mn, mx, def, flags) {}
	};

	class Time : public Scalar
	{
	public:
		Time(const QString& name, ParamTag tag, 
				 TimeUnit::unit internalUnit, double mn, double mx, double def,
				 unsigned int guiUnits, TimeUnit::unit guiDefaultUnit);

		TimeUnit::unit m_internalUnit;
		unsigned int m_guiUnits;
		TimeUnit::unit m_guiUnit;
	};

	class Enum : public Scalar
	{
	public:
		Enum(const QString& name, ParamTag tag, const std::vector<QString>& items, double def);

		std::vector<QString> m_items;
		void itemsChanged();
	};
};

/////////////////////////////////////////////////////////////////////////////

class MachInfo : public Object
{
public:
	QString m_id;
	QString m_name;
	MachineTypeHint::mt m_typehint;
	std::vector< Ptr<PinInfo> > m_inpins, m_outpins;
	Ptr<ParamInfo::Group> m_params;
	unsigned int m_flags;

	MachInfo(const QString& id) : m_id(id), m_flags(0), m_typehint(MachineTypeHint::none) {}
	
	MachInfo* setName(const QString& name)
	{	m_name = name; return this;   }

	MachInfo* setTypeHint(MachineTypeHint::mt typehint)
	{	m_typehint = typehint; return this;   }

	MachInfo* addInPin(const Ptr<PinInfo> pin)
	{	m_inpins.push_back(pin); return this;   }

	MachInfo* addOutPin(const Ptr<PinInfo> pin)
	{	m_outpins.push_back(pin); return this;   }

	MachInfo* setParams(const Ptr<ParamInfo::Group> params)
	{	m_params = params; return this;   }

	MachInfo* addFlags(unsigned int flags)
	{	m_flags |= flags; return this;   }
};
