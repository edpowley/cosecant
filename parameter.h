#pragma once

#include "machine.h"

namespace Parameter
{
	class Scalar;

	struct ParamPinSpec
	{
		ParamPinSpec() {}
		ParamPinSpec(Parameter::Scalar* p, CosecantAPI::TimeUnit::e u = CosecantAPI::TimeUnit::none) 
			: param(p), timeUnit(u) {}

		Parameter::Scalar* param;
		CosecantAPI::TimeUnit::e timeUnit;
	};

	class Base : public Object
	{
		Q_OBJECT

	public:
		Base(const Ptr<Machine>& mac) : m_mac(mac) {}
		Ptr<Machine> getMachine() { return m_mac; }
		QString getName() { return m_name; }
		virtual ParamTag getTag() = 0;

		void setName(const char* name) { m_name = decodeApiString(name); }

		// "Stuff" = the mapping from tags to parameters, and the initial value changes
		virtual void initParamStuff(Machine* mac) = 0;

		virtual int addToParamEditor(QGridLayout* grid, int row) = 0;

		virtual QMenu* populateParamPinMenu(QMenu* menu, QMap<QAction*, ParamPinSpec>& actions) { return NULL; }

		Ptr<ParamPin> getParamPin() { return m_paramPin; }
		void setParamPin(const Ptr<ParamPin>& pin);
		void unsetParamPin() { setParamPin(NULL); }

	signals:
		void signalAddParamPin();
		void signalRemoveParamPin();

	protected:
		Ptr<Machine> m_mac;
		QString m_name;
		Ptr<ParamPin> m_paramPin;
	};

	class Group : public Base
	{
	public:
		Group(const Ptr<Machine>& mac, const ParamGroupInfo* info);
		virtual ParamTag getTag() { return 0; }

		virtual void initParamStuff(Machine* mac);

		virtual int addToParamEditor(QGridLayout* grid, int row);
		void populateParamEditorGrid(QGridLayout* grid);

		virtual QMenu* populateParamPinMenu(QMenu* menu, QMap<QAction*, ParamPinSpec>& actions);

	protected:
		QList< Ptr<Base> > m_params;
	};

	class Scalar : public Base
	{
		Q_OBJECT

	public:
		Scalar(const Ptr<Machine>& mac, ParamTag tag);
		virtual ParamTag getTag() { return m_tag; }
		bool getFreeRange() { return m_freeRange; }

		virtual double sanitise(double v) = 0;

		void setRange(double min, double max, ParamFlags::i flags);
		void setDefault(double def);
		void setState(double state);
		void setScale(ParamScale::e scale);

		double getMin()		{ return m_min; }
		double getMax()		{ return m_max; }
		double getRange()	{ return m_max - m_min; }
		double getDefault()	{ return m_def; }
		double getState()	{ return m_state; }
		ParamScale::e getScale() { return m_scale; }
		
		virtual void initParamStuff(Machine* mac);

		void change(double newval);

		virtual QMenu* populateParamPinMenu(QMenu* menu, QMap<QAction*, ParamPinSpec>& actions);

	signals:
		void valueChanged(double v);

	protected:
		double m_min, m_max, m_def, m_state;
		ParamTag m_tag;
		ParamScale::e m_scale;
		bool m_freeRange;
	};

	class Real : public Scalar
	{
	public:
		Real(const Ptr<Machine>& mac, const RealParamInfo* info);
		virtual int addToParamEditor(QGridLayout* grid, int row);

		virtual double sanitise(double v);
	};

	class Int : public Scalar
	{
	public:
		Int(const Ptr<Machine>& mac, const IntParamInfo* info);
		virtual int addToParamEditor(QGridLayout* grid, int row);

		virtual double sanitise(double v);
	};

	class Time : public Scalar
	{
	public:
		Time(const Ptr<Machine>& mac, const TimeParamInfo* info);
		virtual int addToParamEditor(QGridLayout* grid, int row);

		TimeUnit::e getInternalUnit() { return m_internalUnit; }
		TimeUnit::e getDisplayUnit() { return m_displayUnit; }
		void setDisplayUnit(TimeUnit::e unit) { m_displayUnit = unit; }
		unsigned int getDisplayUnits() { return m_displayUnits; }

		TimeValue getTMin() { return m_tmin; }
		TimeValue getTMax() { return m_tmax; }

		virtual double sanitise(double v);

		virtual QMenu* populateParamPinMenu(QMenu* menu, QMap<QAction*, ParamPinSpec>& actions);

	protected:
		TimeUnit::e m_internalUnit, m_displayUnit;
		unsigned int m_displayUnits;
		TimeValue m_tmin, m_tmax, m_tdef, m_tstate;
	};

	class Enum : public Scalar
	{
	public:
		Enum(const Ptr<Machine>& mac, const EnumParamInfo* info);
		void setItems(const QStringList& items);
		QStringList getItems() { return m_items; }

		virtual int addToParamEditor(QGridLayout* grid, int row);

	protected:
		QStringList m_items;
		virtual double sanitise(double v);
	};
};
