#include "stdafx.h"
#include "common.h"
#include "machine.h"

#include "routing.h"
#include "sequence.h"
#include "song.h"
#include "timeunit_convert.h"
#include "parameditor.h"
#include "cosecantmainwindow.h"
#include "theme.h"
#include "application.h"
#include "callbacks.h"

Machine::Machine()
: m_pos(0,0), m_halfsize(40,25), m_name("Unnamed"), m_parameditor(NULL),
m_routing(NULL), m_perfCount(0),
m_dead(false), m_mi(NULL), m_scriptObject(QScriptValue::NullValue)
{
}

Machine::~Machine()
{
}

static QScriptValue MachineScriptFunctionCaller(QScriptContext* ctx, QScriptEngine* eng)
{
	Machine* mac = dynamic_cast<Machine*>(ctx->thisObject().property("cscMachine").toQObject());
	ASSERT(mac != NULL);
	int id = ctx->callee().data().toInt32();
	return mac->callScriptFunction(ctx, eng, id);
}

void Machine::init()
{
	initInfo();

	m_name = decodeApiString(m_info->defaultName);
	m_colorhint = static_cast<MachineTypeHint::e>(m_info->typeHint);

	initParams(&m_info->params);

	initPins(Pin::in,  m_info->inPins);
	initPins(Pin::out, m_info->outPins);

	if (m_info->script)
	{
		QScriptEngine* se = Application::get().getScriptEngine();
		se->pushContext();

		QScriptValue ctor = se->evaluate(
			m_info->script,
			QString("<Machine 0x%1 script>").arg(reinterpret_cast<quintptr>(this), 0, 16) );
		m_scriptObject = ctor.construct();

		if (!m_scriptObject.isObject())
			m_scriptObject = QScriptValue::NullValue;

		se->clearExceptions();

		if (m_scriptObject.isObject())
		{
			m_scriptFunctionObject = se->newObject();
			m_scriptObject.setProperty("cscFunctions", m_scriptFunctionObject,
				QScriptValue::ReadOnly | QScriptValue::Undeletable);
			m_scriptFunctionObject.setProperty("cscMachine", se->newQObject(this),
				QScriptValue::ReadOnly | QScriptValue::Undeletable);
		}

		se->popContext();
	}

	qDebug() << "m_scriptObject =" << m_scriptObject.toString();

	initImpl();
}

void Machine::addScriptFunction(const QString& name, int id)
{
	if (m_scriptFunctionObject.isObject())
	{
		QScriptEngine* se = Application::get().getScriptEngine();

		QScriptValue func = se->newFunction(MachineScriptFunctionCaller);
		func.setData(id);

		m_scriptFunctionObject.setProperty(name, func, QScriptValue::ReadOnly | QScriptValue::Undeletable);
	}
}

void Machine::setPos(const QPointF& newpos)
{
	m_pos = newpos;
	signalChangePos();
}

QColor Machine::getColor()
{
	return Theme::get().getMachineTypeHintColor(m_colorhint);
}

////////////////////////////////////////////////////////////////////////////////////

void Machine::showParamEditor()
{
	if (m_parameditor)
	{
		m_parameditor->getDock()->setFloating(true);
		m_parameditor->activateWindow();
	}
	else
	{
		CosecantMainWindow* w = CosecantMainWindow::get();

		QDockWidget* dock = new QDockWidget(w);
		dock->setAttribute(Qt::WA_DeleteOnClose);
		dock->setAllowedAreas(Qt::AllDockWidgetAreas);
		ParamEditor* ed = new ParamEditor(this, dock);
		dock->setWidget(ed);
		w->addDockWidget(Qt::LeftDockWidgetArea, dock);
		dock->setFloating(true);
		ed->activateWindow();
	}
}

void Machine::initParams(ParamGroupInfo* group)
{
	m_params = new Parameter::Group(this, group);
	m_params->initParamStuff(this);
}

ParamPin::ParamPin(Parameter::Scalar* param, TimeUnit::e timeUnit)
:	Pin(param->getMachine(), in, SignalType::paramControl, PinFlags::breakOnEvent),
	m_param(param), m_timeUnit(timeUnit)
{
	m_name = tr("Parameter '%1'").arg(m_param->getName());
}

//////////////////////////////////////////////////////////////////////////

Ptr<Sequence::Pattern> Machine::createPattern(double length)
{
	Ptr<Sequence::Pattern> pat = createPatternImpl(length);
	if (pat)
	{
		pat->m_name = QString("%1").arg(m_patterns.size(), 2, 10, QLatin1Char('0'));
	}

	return pat;
}

void Machine::addPattern(const Ptr<Sequence::Pattern>& pat)
{
	m_patterns.push_back(pat);
	pat->added();
}

void Machine::removePattern(const Ptr<Sequence::Pattern>& pat)
{
	vectorEraseFirst(m_patterns, pat);
	pat->removed();
}

QWidget* Machine::createPatternEditorWidget(const Ptr<Sequence::Pattern>& pattern)
{
	if (m_scriptObject.isObject())
	{
		QScriptEngine* se = m_scriptObject.engine();

		QScriptValue peCtor = m_scriptObject.property("cscPatternEditor");
		if (!peCtor.isValid()) throw Error("Script object has no cscPatternEditor property");

		QScriptValue patv = se->newQObject(
			pattern, QScriptEngine::QtOwnership, QScriptEngine::ExcludeDeleteLater);

		QScriptValue pev = peCtor.construct(QScriptValueList() << m_scriptObject << patv);
		if (pev.engine()->hasUncaughtException())
		{
			throw Error(QString("Uncaught exception in cscPatternEditor:\n%1").arg(pev.toString()));
		}

		QWidget* pew = qscriptvalue_cast<QWidget*>(pev);
		if (!pew) throw Error("cscPatternEditor is not a constructor for a QWidget-derived class");

		return pew;
	}

	throw Error("This machine has no script object");
}

//////////////////////////////////////////////////////////////////////////

void Machine::initPins(Pin::Direction direction, const PinInfo** pins)
{
	if (!pins) return;

	std::vector< Ptr<Pin> >& pinvector = (direction == Pin::in) ? m_inpins : m_outpins;
	Pin::Side side = (direction == Pin::in) ? Pin::left : Pin::right;

	int pinnum = 0;
	for (const PinInfo** i = pins; *i; ++i)
	{
		Ptr<Pin> pin = new Pin(this, direction, static_cast<SignalType::e>((*i)->type), (*i)->flags);
		pin->setSide(side);
		pin->setPos(pinnum);
		pin->m_name = (*i)->name;
		pinvector.push_back(pin);
		++ pinnum;
	}

	autoArrangePins();
}

void Machine::addPin(const Ptr<Pin>& pin)
{
	if (pin->m_direction == Pin::in)
		m_inpins.push_back(pin);
	else
		m_outpins.push_back(pin);

	signalAddPin(pin);

	autoArrangePins(pin->getSide());

	if (m_routing) m_routing->signalTopologyChange();
}

void Machine::removePin(const Ptr<Pin>& pin)
{
	vectorEraseFirst(m_inpins, pin);
	vectorEraseFirst(m_outpins, pin);

	signalRemovePin(pin);

	autoArrangePins(pin->getSide());

	if (m_routing) m_routing->signalTopologyChange();
}

void Machine::autoArrangePins()
{
	autoArrangePins(Pin::top);
	autoArrangePins(Pin::right);
	autoArrangePins(Pin::bottom);
	autoArrangePins(Pin::left);
}

void Machine::autoArrangePins(Pin::Side side)
{
	std::multimap< float, Ptr<Pin> > pins;
	BOOST_FOREACH(const Ptr<Pin>& pin, m_inpins)
	{
		if (pin->getSide() == side)
			pins.insert(std::make_pair(pin->getPos(), pin));
	}
	BOOST_FOREACH(const Ptr<Pin>& pin, m_outpins)
	{
		if (pin->getSide() == side)
			pins.insert(std::make_pair(pin->getPos(), pin));
	}

	if (pins.empty())
		return;

	float posstep = 1.0f / float(pins.size());
	float pos = posstep * 0.5f;

	for (std::multimap< float, Ptr<Pin> >::iterator iter = pins.begin(); iter != pins.end(); ++iter)
	{
		iter->second->setPos(pos);
		pos += posstep;
	}

	if (m_routing) m_routing->signalChange();
}

QPointF Pin::getPosOffset()
{
	switch (m_side)
	{
	case left:
	case right:
		return QPointF(
			(m_side == left) ? -1.0f : +1.0f,
			(m_pos - 0.5f) * 2.0f
		);

	case top:
	case bottom:
		return QPointF(
			(m_pos - 0.5f) * 2.0f,
			(m_side == top) ? -1.0f : +1.0f
		);

	default:
		return QPointF(-1,-1);
	}
}

QPointF Pin::getAbsPos()
{
	return m_machine->m_pos + multElementWise(m_machine->m_halfsize, getPosOffset());
}
