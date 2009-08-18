#pragma once

#include "workbuffer.h"
#include "sequence.h"
#include "cosecant_api.h"

////////////////////////////////////////////////////////////////////////

class SongLoadContext;

namespace Parameter { class Base; class Group; class Scalar; };

class Machine;
class Connection;

class Pin : public ObjectWithUuid
{
	Q_OBJECT

public:
	enum Direction {in, out};

	Pin(Machine* machine, Direction direction, SignalType::e type)
		: m_machine(machine), m_direction(direction), m_type(type) {}

	// Not a smart ptr: the circular references would give me a headache
	Machine* m_machine;

	Direction m_direction;

	enum Side {top, right, bottom, left};
	float getPos() { return m_pos; }
	void setPos(float pos) { m_pos  = pos; signalPosChanged(); }
	Side getSide() { return m_side; }
	void setSide(Side side) { m_side = side; signalPosChanged(); }
	
	QString m_name;
	SignalType::e m_type;

	Ptr<WorkBuffer::Base> m_buffer;

	QPointF getPosOffset();
	QPointF getAbsPos();
	double getRotation() { return (int)m_side * 90.0; }

	QList< Ptr<Connection> > m_connections;

	virtual void load(SongLoadContext& ctx, const QDomElement& el);
	virtual QDomElement save(QDomDocument& doc);

signals:
	void signalPosChanged();

protected:
	Side m_side;
	float m_pos;
};

class ParamPin : public Pin
{
public:
	ParamPin(Parameter::Scalar* param, TimeUnit::e timeUnit);

	Parameter::Scalar* getParam() { return m_param; }
	TimeUnit::e getTimeUnit() { return m_timeUnit; }

	virtual QDomElement save(QDomDocument& doc);

protected:
	Parameter::Scalar* m_param;
	TimeUnit::e m_timeUnit;
};

///////////////////////////////////////////////////////////////////////

class Connection : public ObjectWithUuid
{
public:
	Connection(const Ptr<Pin>& pin1, const Ptr<Pin>& pin2, bool feedback)
		: m_pin1(pin1), m_pin2(pin2), m_feedback(feedback) {}

	bool m_feedback;

	Ptr<Pin> getPin1() { return m_pin1; }
	Ptr<Pin> getPin2() { return m_pin2; }

	QDomElement save(QDomDocument& doc);

	Ptr<DelayLine::Base> m_feedbackDelayLine;

protected:
	Ptr<Pin> m_pin1, m_pin2;
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

	void init();

	QString getName() { return m_name; }
	void setName(const QString& name) { m_name = name; signalRename(name); }

	MachineTypeHint::e getColorHint() { return m_colorhint; }
	void setColorHint(MachineTypeHint::e hint) { m_colorhint = hint; signalChangeAppearance(); }

	QColor getColor();

	MachineInfo* getInfo() { return m_info; }

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

	void signalAddPin(const Ptr<Pin>& pin);
	void signalRemovePin(const Ptr<Pin>& pin);

public:
	void added() { signalAdd(); }
	void removed() { signalRemove(); }

public:
	std::vector< Ptr<Pin> > m_inpins, m_outpins;
	Ptr<Pin> m_noteTriggerPin;

	Ptr<Parameter::Group> m_params;
	QHash< ParamTag, Ptr<Parameter::Base> > m_paramMap;

	ParamEditor* m_parameditor;
	void showParamEditor();

	QMutex m_paramChangesMutex;
	std::map<ParamTag, double> m_paramChanges;

	void addPin(const Ptr<Pin>& pin);
	void removePin(const Ptr<Pin>& pin);

	void autoArrangePins(Pin::Side side);
	void autoArrangePins();

	__int64 m_perfCount;

	virtual void changeParam(ParamTag tag, double value) = 0;
	virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe) = 0;

	std::map<void*, void*> m_noteIdMap;

	virtual void load(SongLoadContext& ctx, const QDomElement& el);
	QDomElement save(QDomDocument& doc);

	Ptr<Sequence::Pattern> createPattern(double length);
	std::vector< Ptr<Sequence::Pattern> > m_patterns;
	void addPattern(const Ptr<Sequence::Pattern>& pat);
	void removePattern(const Ptr<Sequence::Pattern>& pat);

	virtual QWidget* createPatternEditorWidget(const Ptr<Sequence::Pattern>& pattern);

	void addScriptFunction(const QString& name, int id);
	virtual QScriptValue callScriptFunction(QScriptContext* ctx, QScriptEngine* eng, int id)
	{ return QScriptValue::NullValue; }

protected:
	CosecantAPI::Mi* m_mi;

	virtual void initImpl() = 0;
	void initPins(Pin::Direction direction, const PinInfo** pininfos);
	void initParams(ParamGroupInfo* group);

	MachineInfo* m_info;
	virtual void initInfo() = 0;

	QString m_id;
	QString m_name;
	CosecantAPI::MachineTypeHint::e m_colorhint;

	QScriptValue m_scriptObject, m_scriptFunctionObject;

	virtual Ptr<Sequence::Pattern> createPatternImpl(double length) { return NULL; }

	void loadPins(SongLoadContext& ctx, Pin::Direction direction, const QList<QDomElement>& els);
};

//////////////////////////////////////////////////////////////////////////

class MachineFactory : public Object
{
public:
	static bool add(const QString& id, MachineFactory* factory);
	static Ptr<MachineFactory> get(const QString& id);
	ERROR_CLASS(BadIdError);

	MachineFactory(const QString& id) : m_id(id) {}

	Ptr<Machine> createMachine();
	virtual QString getDesc() = 0;

	static std::map<QString, Ptr<MachineFactory> > s_factories;

protected:
	virtual Ptr<Machine> createMachineImpl() = 0;

	QString m_id;
};
