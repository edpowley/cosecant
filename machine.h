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

	void load(class SongLoadContext& ctx, xmlpp::Element* el);
	void save(xmlpp::Element* el);
};

///////////////////////////////////////////////////////////////////////

class Connection : public ObjectWithUuid
{
public:
	Connection(const Ptr<Pin>& pin1, const Ptr<Pin>& pin2, bool feedback)
		: m_pin1(pin1), m_pin2(pin2), m_feedback(feedback) {}

	Connection(SongLoadContext& ctx, xmlpp::Element* el);

	bool m_feedback;

	Ptr<Pin> getPin1() { return m_pin1; }
	Ptr<Pin> getPin2() { return m_pin2; }

	void save(xmlpp::Element* el);

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

	Ptr<MachInfo> m_info;
	QString m_name;
	CosecantAPI::MachineTypeHint::mt m_colorhint;
	QPointF m_pos, m_halfsize;
	Routing* m_routing;

	boost::recursive_timed_mutex m_mutex;

	bool m_dead;
	QString m_deadWhy;

signals:
	void signalAdd();
	void signalRemove();

public:
	void added() { signalAdd(); }
	void removed() { signalRemove(); }

public:
	std::vector< Ptr<Pin> > m_inpins, m_outpins;
	Ptr<Pin> m_noteTriggerPin;

	Ptr<ParamInfo::Group> m_params;
	Ptr<ParamInfo::Base> getParam(ParamTag tag);

	ParamEditor* m_parameditor;

	boost::mutex m_paramChangesMutex;
	std::map<ParamTag, ParamValue> m_paramChanges, m_paramStates;
//	std::map<ParamTag, sigc::signal<void> > m_signalParamChange;

	virtual void changeParam(ParamTag tag, ParamValue value) = 0;
	virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe) = 0;
	virtual void playPattern(const Ptr<Sequence::Track>& track, const Ptr<Sequence::Event>& ev, double pos) {}

	void addPin(const Ptr<Pin>& pin);
	void removePin(const Ptr<Pin>& pin);

	void autoArrangePins(Pin::Side side);
	void autoArrangePins();

	__int64 m_perfCount;

	std::map<void*, void*> m_noteIdMap;

	virtual void* noteOn(double note, double vel) { return NULL; }
	virtual void noteOff(void* note) {}

	virtual void load(class SongLoadContext& ctx, xmlpp::Element* el);
	void save(xmlpp::Element* el);

	Ptr<Sequence::Pattern> createPattern(double length);
	Ptr<Sequence::Pattern> createPattern(class SongLoadContext& ctx, xmlpp::Element* el);
	std::vector< Ptr<Sequence::Pattern> > m_patterns;
	void addPattern(const Ptr<Sequence::Pattern>& pat);
	void removePattern(const Ptr<Sequence::Pattern>& pat);

	virtual class PatternEditor* createPatternEditor(const Ptr<Sequence::Pattern>& pattern);

	boost::mutex m_playingPatternsMutex;
	struct EventPlayRec
	{
		Ptr<Sequence::Event> m_event;
		double m_pos;
		EventPlayRec(const Ptr<Sequence::Event>& ev, double pos) : m_event(ev), m_pos(pos) {}
	};
	std::vector<EventPlayRec> m_playingEvents;

protected:
	void init(const Ptr<MachInfo>& info);
	void initPins(Pin::Direction direction, const std::vector< Ptr<PinInfo> >& pininfos);
	void initParamStates(const Ptr<ParamInfo::Group>& group);

	virtual Ptr<Sequence::Pattern> newPattern(double length);
};

//////////////////////////////////////////////////////////////////////////

class DummyMachine : public Machine
{
public:
	DummyMachine()
	{
		m_dead = true;
		m_deadWhy = "This is a placeholder for a machine which you do not have installed.";
	}

	virtual void changeParam(ParamTag tag, ParamValue value) {}
	virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe) {}

	static Ptr<MachInfo> getInfo()
	{
		return (new MachInfo("builtin/dummy"))->setName("Dummy");
	}

	virtual void load(class SongLoadContext& ctx, xmlpp::Element* el);
};

////////////////////////////////////////////////////////////////////////

class MachineFactory : public Object
{
public:
	static bool add(const QString& id, MachineFactory* factory);
	static Ptr<MachineFactory> get(const QString& id);
	ERROR_CLASS(BadIdError);

	Ptr<Machine> createMachine();

	virtual Ptr<MachInfo> getMachInfo() = 0;

	static std::map<QString, Ptr<MachineFactory> > s_factories;

protected:
	virtual Ptr<Machine> createMachineImpl() = 0;
};
