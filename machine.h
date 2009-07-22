#pragma once

#include "machinfo.h"
#include "workbuffer.h"
#include "sequence.h"
#include "cosecant_api.h"

////////////////////////////////////////////////////////////////////////

class Machine;
class Connection;

class Pin : public ObjectWithUuid
{
public:
	enum Direction {in, out};

	Pin(Machine* machine, Direction direction, SignalType::st type)
		: m_machine(machine), m_direction(direction), m_type(type), m_isParamPin(false) {}

	// Not a smart ptr: the circular references would give me a headache
	Machine* m_machine;

	Direction m_direction;

	enum Side {top, right, bottom, left};
	Side m_side;
	float m_pos;
	QString m_name;
	SignalType::st m_type;
	bool m_isParamPin;
	ParamTag m_paramTag;

	Ptr<WorkBuffer::Base> m_buffer;

	QPointF getPosOffset();
	QPointF getAbsPos();
	double getRotation() { return (int)m_side * 90.0; }

	std::vector< Ptr<Connection> > m_connections;

	void load(class SongLoadContext& ctx, const QDomElement& el);
	void save(const QDomElement& el);
};

///////////////////////////////////////////////////////////////////////

class Connection : public ObjectWithUuid
{
public:
	Connection(const Ptr<Pin>& pin1, const Ptr<Pin>& pin2, bool feedback)
		: m_pin1(pin1), m_pin2(pin2), m_feedback(feedback) {}

	Connection(SongLoadContext& ctx, const QDomElement& el);

	bool m_feedback;

	Ptr<Pin> getPin1() { return m_pin1; }
	Ptr<Pin> getPin2() { return m_pin2; }

	void save(const QDomElement& el);

	Ptr<DelayLine::Base> m_feedbackDelayLine;

protected:
	Ptr<Pin> m_pin1, m_pin2;
};

///////////////////////////////////////////////////////////////////////

namespace Parameter
{
	class Base : public Object
	{
		Q_OBJECT

	public:
		Base(const Ptr<Machine>& mac) : m_mac(mac) {}
		Ptr<Machine> getMachine() { return m_mac; }
		QString getName() { return m_name; }

		void setName(const QString& name) { m_name = name; }

		// "Stuff" = the mapping from tags to parameters, and the initial value changes
		virtual void initParamStuff(Machine* mac) = 0;

		virtual int addToParamEditor(QGridLayout* grid, int row) = 0;

	protected:
		Ptr<Machine> m_mac;
		QString m_name;
	};

	class Group : public Base
	{
	public:
		Group(const Ptr<Machine>& mac, InfoImpl::ParamInfo::Group* info);

		virtual void initParamStuff(Machine* mac);

		virtual int addToParamEditor(QGridLayout* grid, int row);
		void populateParamEditorGrid(QGridLayout* grid);

	protected:
		std::vector< Ptr<Base> > m_params;
	};

	class Scalar : public Base
	{
		Q_OBJECT

	public:
		Scalar(const Ptr<Machine>& mac, ParamTag tag);

		void setRange(double min, double max);
		void setDefault(double def);
		void setState(double state);
		void setFlags(unsigned int flags);

		double getMin()		{ return m_min; }
		double getMax()		{ return m_max; }
		double getRange()	{ return m_max - m_min; }
		double getDefault()	{ return m_def; }
		double getState()	{ return m_state; }
		
		bool hasFlag(unsigned int f) { return (m_flags & f) == f; }

		virtual void initParamStuff(Machine* mac);

		void change(double newval);

	signals:
		void valueChanged(double v);

	protected:
		double m_min, m_max, m_def, m_state;
		ParamTag m_tag;
		unsigned int m_flags;

		virtual double sanitise(double v) = 0;
	};

	class Real : public Scalar
	{
	public:
		Real(const Ptr<Machine>& mac, InfoImpl::ParamInfo::Real* info);
		virtual int addToParamEditor(QGridLayout* grid, int row);

	protected:
		virtual double sanitise(double v);
	};

	class Int : public Scalar
	{
	public:
		Int(const Ptr<Machine>& mac, InfoImpl::ParamInfo::Int* info);
		virtual int addToParamEditor(QGridLayout* grid, int row);

	protected:
		virtual double sanitise(double v);
	};

	class Time : public Scalar
	{
	public:
		Time(const Ptr<Machine>& mac, InfoImpl::ParamInfo::Time* info);
		virtual int addToParamEditor(QGridLayout* grid, int row);

		TimeUnit::unit getInternalUnit() { return m_internalUnit; }
		TimeUnit::unit getDisplayUnit() { return m_displayUnit; }
		void setDisplayUnit(TimeUnit::unit unit) { m_displayUnit = unit; }
		unsigned int getDisplayUnits() { return m_displayUnits; }

		TimeValue getTMin() { return m_tmin; }
		TimeValue getTMax() { return m_tmax; }

	protected:
		TimeUnit::unit m_internalUnit, m_displayUnit;
		unsigned int m_displayUnits;
		TimeValue m_tmin, m_tmax, m_tdef, m_tstate;
		virtual double sanitise(double v);
	};

	class Enum : public Scalar
	{
	public:
		Enum(const Ptr<Machine>& mac, InfoImpl::ParamInfo::Enum* info);
		void setItems(const QStringList& items);
		QStringList getItems() { return m_items; }

		virtual int addToParamEditor(QGridLayout* grid, int row);

	protected:
		QStringList m_items;
		virtual double sanitise(double v);
	};
};

///////////////////////////////////////////////////////////////////////

class ParamEditor;
class Routing;

class Machine : public ObjectWithUuid
{
	friend class MachineFactory;

	Q_OBJECT

public:
	Machine();
	virtual ~Machine();

	virtual CosecantAPI::Mi* createMi(CosecantAPI::Callbacks* cb) = 0;
	CosecantAPI::Mi* getMi() { return m_mi; }

	QString getName() { return m_name; }
	void setName(const QString& name) { m_name = name; signalRename(name); }

	MachineTypeHint::mt getColorHint() { return m_colorhint; }
	void setColorHint(MachineTypeHint::mt hint) { m_colorhint = hint; signalChangeAppearance(); }

	QColor getColor();

	InfoImpl::MachineInfo* m_info;
	QPointF m_pos, m_halfsize;
	Routing* m_routing;

	void setPos(const QPointF& newpos);

	QMutex_Recursive m_mutex;

	bool m_dead;
	QString m_deadWhy;

signals:
	void signalAdd();
	void signalRemove();
	void signalChangePos();

	void signalRename(const QString& newname);
	void signalChangeAppearance();

public:
	void added() { signalAdd(); }
	void removed() { signalRemove(); }

public:
	std::vector< Ptr<Pin> > m_inpins, m_outpins;
	Ptr<Pin> m_noteTriggerPin;

	Ptr<Parameter::Group> m_params;
	MyMap< ParamTag, Ptr<Parameter::Base> > m_paramMap;

	ParamEditor* m_parameditor;
	void showParamEditor();

	QMutex m_paramChangesMutex;
	std::map<ParamTag, ParamValue> m_paramChanges;

	void addPin(const Ptr<Pin>& pin);
	void removePin(const Ptr<Pin>& pin);

	void autoArrangePins(Pin::Side side);
	void autoArrangePins();

	__int64 m_perfCount;

	std::map<void*, void*> m_noteIdMap;

//	virtual void load(class SongLoadContext& ctx, const QDomElement& el);
	void save(const QDomElement& el);

	Ptr<Sequence::Pattern> createPattern(double length);
	Ptr<Sequence::Pattern> createPattern(class SongLoadContext& ctx, const QDomElement& el);
	std::vector< Ptr<Sequence::Pattern> > m_patterns;
	void addPattern(const Ptr<Sequence::Pattern>& pat);
	void removePattern(const Ptr<Sequence::Pattern>& pat);

	virtual class PatternEditor* createPatternEditor(const Ptr<Sequence::Pattern>& pattern);

protected:
	CosecantAPI::Mi* m_mi;

	void init(InfoImpl::MachineInfo* info);
	void initPins(Pin::Direction direction, const std::vector<InfoImpl::PinInfo*>& pininfos);
	void initParams(InfoImpl::ParamInfo::Group* group);

	QString m_name;
	CosecantAPI::MachineTypeHint::mt m_colorhint;
};

//////////////////////////////////////////////////////////////////////////

class MachineFactory : public Object
{
public:
	static bool add(const QString& id, MachineFactory* factory);
	static Ptr<MachineFactory> get(const QString& id);
	ERROR_CLASS(BadIdError);

	Ptr<Machine> createMachine();

	virtual InfoImpl::MachineInfo* getMachInfo() = 0;

	static std::map<QString, Ptr<MachineFactory> > s_factories;

protected:
	virtual Ptr<Machine> createMachineImpl() = 0;
};
