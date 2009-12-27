#include "stdafx.h"
#include "common.h"
#include "machine.h"

#include "routing.h"
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
m_dead(false), m_mi(NULL),
m_eventBuffer(new WorkBuffer::Events)
{
}

Machine::~Machine()
{
}

void Machine::init()
{
	initInfo();

	m_name = decodeApiString(m_info->defaultName);
	m_colorhint = static_cast<MachineTypeHint::e>(m_info->typeHint);

	initParams(&m_info->params);

	initPins(Pin::in,  m_info->inPins);
	initPins(Pin::out, m_info->outPins);

	initImpl();
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

void Machine::initPins(Pin::Direction direction, const PinInfo* const* pins)
{
	if (!pins) return;

	std::vector< Ptr<Pin> >& pinvector = (direction == Pin::in) ? m_inpins : m_outpins;
	Pin::Side side = (direction == Pin::in) ? Pin::left : Pin::right;

	int pinnum = 0;
	for (const PinInfo* const* i = pins; *i; ++i)
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
