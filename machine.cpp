#include "stdafx.h"
#include "common.h"
#include "machine.h"
#include "routing.h"
#include "sequence.h"
#include "song.h"
#include "spattern.h"
//#include "spatterneditor.h"

Machine::Machine()
: m_pos(0,0), m_halfsize(50,25), m_name("Unnamed"), m_parameditor(NULL),
m_routing(NULL), m_perfCount(0),
m_dead(false)
{
}

Machine::~Machine()
{
}

void Machine::init(const Ptr<MachInfo>& info)
{
	m_info = info;
	m_name = info->m_name;
	m_colorhint = info->m_typehint;

	m_params = info->m_params;
	initParamStates(m_params);
	m_paramChanges = m_paramStates;

	initPins(Pin::in,  info->m_inpins);
	initPins(Pin::out, info->m_outpins);

	if (info->m_flags & MachineFlags::hasNoteTrigger)
	{
		Ptr<Pin> pin = new Pin(this, Pin::in, SignalType::noteTrigger);
		pin->m_side = Pin::top;
		pin->m_pos = 0.5f;
		pin->m_name = "Note trigger";
		m_inpins.push_back(pin);
		m_noteTriggerPin = pin;
	}
}

void Machine::initParamStates(const Ptr<ParamInfo::Group>& group)
{
	if (!group) return;

	BOOST_FOREACH(ParamInfo::Base* p, group->m_params)
	{
		if (ParamInfo::Group* pg = dynamic_cast<ParamInfo::Group*>(p))
		{
			initParamStates(pg);
		}
		else if (ParamInfo::Scalar* ps = dynamic_cast<ParamInfo::Scalar*>(p))
		{
			m_paramStates[ps->m_tag] = ps->m_def;
		}
	}
}

void Machine::setPos(const QPointF& newpos)
{
	m_pos = newpos;
	signalChangePos();
}

//////////////////////////////////////////////////////////////////////////

Ptr<Sequence::Pattern> Machine::createPattern(double length)
{
	Ptr<Sequence::Pattern> pat = newPattern(length);
	pat->m_name = QString("%1").arg(m_patterns.size(), 2, 10, QLatin1Char('0'));
	return pat;
}

Ptr<Sequence::Pattern> Machine::newPattern(double length)
{
	return new Spattern(this, length);
}

void Machine::addPattern(const Ptr<Sequence::Pattern>& pat)
{
	m_patterns.push_back(pat);
	pat->added();
	Song::get().m_sequence->trigger_signalMachinePatternsChange(this);
}

void Machine::removePattern(const Ptr<Sequence::Pattern>& pat)
{
	vectorEraseFirst(m_patterns, pat);
	pat->removed();
	Song::get().m_sequence->trigger_signalMachinePatternsChange(this);
}

PatternEditor* Machine::createPatternEditor(const Ptr<Sequence::Pattern>& pattern)
{
//	return new SpatternEditor(dynamic_cast<Spattern*>(pattern.c_ptr()));
	return NULL;
}

//////////////////////////////////////////////////////////////////////////

void Machine::initPins(Pin::Direction direction, const std::vector< Ptr<PinInfo> >& pins)
{
	std::vector< Ptr<Pin> >& pinvector = (direction == Pin::in) ? m_inpins : m_outpins;
	Pin::Side side = (direction == Pin::in) ? Pin::left : Pin::right;

	size_t npins = pins.size();
	float posstep = 1.0f / (float)(npins + 1);
	float pos = posstep;

	for (std::vector< Ptr<PinInfo> >::const_iterator i = pins.begin(); i != pins.end(); ++i)
	{
		Ptr<Pin> pin = new Pin(this, direction, (*i)->m_type);
		pin->m_side = side;
		pin->m_pos = pos; pos += posstep;
		pin->m_name = (*i)->m_name;
		pinvector.push_back(pin);
	}

	autoArrangePins();
}

void Machine::addPin(const Ptr<Pin>& pin)
{
	if (pin->m_direction == Pin::in)
		m_inpins.push_back(pin);
	else
		m_outpins.push_back(pin);

	autoArrangePins(pin->m_side);

	if (m_routing) m_routing->signalTopologyChange();
}

void Machine::removePin(const Ptr<Pin>& pin)
{
	vectorEraseFirst(m_inpins, pin);
	vectorEraseFirst(m_outpins, pin);

	autoArrangePins(pin->m_side);

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
		if (pin->m_side == side)
			pins.insert(std::make_pair(pin->m_pos, pin));
	}
	BOOST_FOREACH(const Ptr<Pin>& pin, m_outpins)
	{
		if (pin->m_side == side)
			pins.insert(std::make_pair(pin->m_pos, pin));
	}

	if (pins.empty())
		return;

	float posstep = 1.0f / float(pins.size());
	float pos = posstep * 0.5f;

	for (std::multimap< float, Ptr<Pin> >::iterator iter = pins.begin(); iter != pins.end(); ++iter)
	{
		iter->second->m_pos = pos;
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
