#pragma once

#include "cosecant_api.h"

namespace InfoImpl
{
	namespace ParamInfo
	{
		class Group : public CosecantAPI::ParamInfo::Group
		{
		public:
			virtual Group* copy() { THROW_ERROR(Error, "Not yet implemented"); }
			virtual Group* setName(const char* name) { m_name = name; return this; }
			virtual Group* setTagMask(CosecantAPI::ParamTag mask) { m_tagMask = mask; return this; }
			virtual Group* addParam(CosecantAPI::ParamInfo::Base* param)
			{ m_params.push_back(param); return this; }

			QString m_name;
			CosecantAPI::ParamTag m_tagMask;
			std::vector<CosecantAPI::ParamInfo::Base*> m_params;
		};

		class Real : public CosecantAPI::ParamInfo::Real
		{
		public:
			Real(CosecantAPI::ParamTag tag) : m_tag(tag), m_min(0), m_max(1), m_def(0), m_flags(0) {}

			virtual Real* copy() { return new Real(*this); }

			virtual Real* setName(const char* name) { m_name = name; return this; }
			virtual Real* setTag(CosecantAPI::ParamTag tag) { m_tag = tag; return this; }
			virtual Real* setRange(CosecantAPI::ParamValue mn, CosecantAPI::ParamValue mx)
			{ m_min = mn; m_max = mx; return this; }
			virtual Real* setDefault(CosecantAPI::ParamValue def) { m_def = def; return this; }
			virtual Real* addFlags(unsigned int flags) { m_flags |= flags; return this; }

			QString m_name;
			CosecantAPI::ParamTag m_tag;
			CosecantAPI::ParamValue m_min, m_max, m_def;
			unsigned int m_flags;
		};

		class Int : public CosecantAPI::ParamInfo::Int
		{
		public:
			Int(CosecantAPI::ParamTag tag) : m_tag(tag), m_min(0), m_max(1), m_def(0) {}

			virtual Int* copy() { return new Int(*this); }
			virtual Int* setName(const char* name) { m_name = name; return this; }
			virtual Int* setTag(CosecantAPI::ParamTag tag) { m_tag = tag; return this; }
			virtual Int* setRange(int mn, int mx) { m_min = mn; m_max = mx; return this; }
			virtual Int* setDefault(int def) { m_def = def; return this; }

			QString m_name;
			CosecantAPI::ParamTag m_tag;
			int m_min, m_max, m_def;
		};

		class Time : public CosecantAPI::ParamInfo::Time
		{
		public:
			Time(CosecantAPI::ParamTag tag)
				: m_tag(tag),
				m_internalUnit(CosecantAPI::TimeUnit::seconds),
				m_defaultDisplayUnit(CosecantAPI::TimeUnit::seconds),
				m_displayUnits(0)
			{}

			virtual Time* copy() { return new Time(*this); }

			virtual Time* setName(const char* name) { m_name = name; return this; }
			virtual Time* setTag(CosecantAPI::ParamTag tag) { m_tag = tag; return this; }
			virtual Time* setRange(CosecantAPI::TimeValue mn, CosecantAPI::TimeValue mx)
			{ m_min = mn; m_max = mx; return this; }
			virtual Time* setDefault(CosecantAPI::TimeValue def) { m_def = def; return this; }

			virtual Time* setInternalUnit(CosecantAPI::TimeUnit::unit unit)
			{ m_internalUnit = unit; return this; }
			virtual Time* addDisplayUnits(unsigned int units)
			{ m_displayUnits |= units; return this; }
			virtual Time* setDefaultDisplayUnit(CosecantAPI::TimeUnit::unit unit)
			{ m_defaultDisplayUnit = unit; return this; }

			QString m_name;
			CosecantAPI::ParamTag m_tag;
			CosecantAPI::TimeValue m_min, m_max, m_def;
			CosecantAPI::TimeUnit::unit m_internalUnit, m_defaultDisplayUnit;
			unsigned int m_displayUnits;
		};

		class Enum : public CosecantAPI::ParamInfo::Enum
		{
		public:
			Enum(CosecantAPI::ParamTag tag) : m_tag(tag) {}
			virtual Enum* copy() { return new Enum(*this); }

			virtual Enum* setName(const char* name) { m_name = name; return this; }
			virtual Enum* setTag(CosecantAPI::ParamTag tag) { m_tag = tag; return this; }
			virtual Enum* addItems(char separator, const char* text);
			virtual Enum* setDefault(int def) { m_def = def; return this; }

			Enum* setItems(const QStringList& items) { m_items = items; return this; }

			QString m_name;
			CosecantAPI::ParamTag m_tag;
			QStringList m_items;
			int m_def;
		};
	};

	class PinInfo : public CosecantAPI::PinInfo
	{
	public:
		virtual PinInfo* setName(const char* name) { m_name = name; return this; }
		virtual PinInfo* setType(CosecantAPI::SignalType::st type) { m_type = type; return this; }

		QString m_name;
		CosecantAPI::SignalType::st m_type;
	};

	class MachineInfo : public CosecantAPI::MachineInfo
	{
	public:
		MachineInfo()
			: m_typeHint(CosecantAPI::MachineTypeHint::none), m_flags(0)
		{}

		virtual MachineInfo* setName(const char* name) { m_name = name; return this; }
		virtual MachineInfo* setTypeHint(CosecantAPI::MachineTypeHint::mt type) { m_typeHint = type; return this; }
		virtual MachineInfo* addFlags(unsigned int flags) { m_flags |= flags; return this; }

		virtual MachineInfo* addInPin (CosecantAPI::PinInfo* pin) { addPin(pin, m_inpins);  return this; }
		virtual MachineInfo* addOutPin(CosecantAPI::PinInfo* pin) { addPin(pin, m_outpins); return this; }

		virtual ParamInfo::Group* getParams() { return &m_params; }

	public:
		QString m_name;
		ParamInfo::Group m_params;
		CosecantAPI::MachineTypeHint::mt m_typeHint;
		unsigned int m_flags;

		std::vector<PinInfo*> m_inpins, m_outpins;

	protected:
		void addPin(CosecantAPI::PinInfo* pin, std::vector<PinInfo*>& pins);
	};

	class InfoCallbacks : public CosecantAPI::InfoCallbacks
	{
	public:
		virtual unsigned int getHostVersion() { return CosecantAPI::version; }

		virtual PinInfo* createPin() { return new PinInfo; }

		virtual ParamInfo::Group* createParamGroup() { return new ParamInfo::Group; }
		virtual ParamInfo::Real* createRealParam(CosecantAPI::ParamTag tag) { return new ParamInfo::Real(tag); }
		virtual ParamInfo::Int*  createIntParam (CosecantAPI::ParamTag tag) { return new ParamInfo::Int (tag); }
		virtual ParamInfo::Time* createTimeParam(CosecantAPI::ParamTag tag) { return new ParamInfo::Time(tag); }
		virtual ParamInfo::Enum* createEnumParam(CosecantAPI::ParamTag tag) { return new ParamInfo::Enum(tag); }
	};
};
