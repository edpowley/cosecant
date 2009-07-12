#include "stdafx.h"
#include "common.h"
#include "machine.h"
#include "routing.h"
#include "sequence.h"
#include "song.h"
#include "spattern.h"
//#include "spatterneditor.h"
#include "timeunit_convert.h"
#include "parameditor.h"
#include "cosecantmainwindow.h"
#include "theme.h"

Machine::Machine()
: m_pos(0,0), m_halfsize(50,25), m_name("Unnamed"), m_parameditor(NULL),
m_routing(NULL), m_perfCount(0),
m_dead(false), m_mi(NULL)
{
}

Machine::~Machine()
{
}

void Machine::init(InfoImpl::MachineInfo* info)
{
	m_info = info;
	m_name = info->m_name;
	m_colorhint = info->m_typeHint;

	initParams(&info->m_params);

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

void Machine::initParams(InfoImpl::ParamInfo::Group* group)
{
	m_params = new Parameter::Group(this, group);
	m_params->initParamStuff(this);
}

Parameter::Group::Group(const Ptr<Machine>& mac, InfoImpl::ParamInfo::Group* info)
: Base(mac)
{
	setName(info->m_name);

	BOOST_FOREACH(CosecantAPI::ParamInfo::Base* p, info->m_params)
	{
#		define MY_IF_MACRO(NAME) \
			if (InfoImpl::ParamInfo::NAME* q = dynamic_cast<InfoImpl::ParamInfo::NAME*>(p)) \
				m_params.push_back(new NAME(mac, q));

		MY_IF_MACRO(Group)
		else MY_IF_MACRO(Real)
		else MY_IF_MACRO(Int)
		else MY_IF_MACRO(Time)
		else MY_IF_MACRO(Enum)

#		undef MY_IF_MACRO
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
: Base(mac), m_tag(tag), m_min(0), m_max(1), m_def(0), m_state(0), m_flags(0)
{
}

void Parameter::Scalar::initParamStuff(Machine* mac)
{
	mac->m_paramMap[m_tag] = this;
	mac->m_paramChanges[m_tag] = m_state;
}

void Parameter::Scalar::setRange(double min, double max)
{
	m_min = min;
	m_max = max;
}

void Parameter::Scalar::setDefault(double def)
{
	m_def = sanitise(def);
}

void Parameter::Scalar::setState(double state)
{
	m_state = sanitise(state);
}

void Parameter::Scalar::setFlags(unsigned int flags)
{
	m_flags = flags;
}

void Parameter::Scalar::change(double newval)
{
	newval = sanitise(newval);
	if (newval != m_state)
	{
		setState(newval);
		valueChanged(newval);

		boost::unique_lock<boost::mutex> lock(m_mac->m_paramChangesMutex);
		m_mac->m_paramChanges[m_tag] = newval;
	}
}

Parameter::Real::Real(const Ptr<Machine>& mac, InfoImpl::ParamInfo::Real* info)
: Scalar(mac, info->m_tag)
{
	setName(info->m_name);
	setFlags(info->m_flags);
	setRange(info->m_min, info->m_max);
	setDefault(info->m_def);
	setState(info->m_def);
}

double Parameter::Real::sanitise(double v)
{
	return clamp(v, m_min, m_max);
}

Parameter::Int::Int(const Ptr<Machine>& mac, InfoImpl::ParamInfo::Int* info)
: Scalar(mac, info->m_tag)
{
	setName(info->m_name);
	setRange(info->m_min, info->m_max);
	setDefault(info->m_def);
	setState(info->m_def);
}

double Parameter::Int::sanitise(double v)
{
	return clamp(floor(v+0.5), m_min, m_max);
}

Parameter::Time::Time(const Ptr<Machine>& mac, InfoImpl::ParamInfo::Time* info)
:	Scalar(mac, info->m_tag),
	m_tmin(info->m_min), m_tmax(info->m_max), m_tdef(info->m_def), m_tstate(info->m_def),
	m_internalUnit(info->m_internalUnit), m_displayUnit(info->m_defaultDisplayUnit),
	m_displayUnits(info->m_displayUnits)
{
	setName(info->m_name);
	setRange(ConvertTimeUnits(m_tmin, m_internalUnit), ConvertTimeUnits(m_tmax, m_internalUnit));
	setDefault(ConvertTimeUnits(m_tdef, m_internalUnit));
	setState(getDefault());
}

double Parameter::Time::sanitise(double v)
{
	return v;
}

Parameter::Enum::Enum(const Ptr<Machine>& mac, InfoImpl::ParamInfo::Enum* info)
:	Scalar(mac, info->m_tag)
{
	setName(info->m_name);
	setItems(info->m_items);
	setDefault(info->m_def);
	setState(info->m_def);
}

double Parameter::Enum::sanitise(double v)
{
	return clamp(floor(v+0.5), m_min, m_max);
}

void Parameter::Enum::setItems(const QStringList& items)
{
	m_items = items;
	setRange(0, m_items.length()-1);
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
}

void Machine::removePattern(const Ptr<Sequence::Pattern>& pat)
{
	vectorEraseFirst(m_patterns, pat);
	pat->removed();
}

PatternEditor* Machine::createPatternEditor(const Ptr<Sequence::Pattern>& pattern)
{
//	return new SpatternEditor(dynamic_cast<Spattern*>(pattern.c_ptr()));
	return NULL;
}

//////////////////////////////////////////////////////////////////////////

void Machine::initPins(Pin::Direction direction, const std::vector<InfoImpl::PinInfo*>& pins)
{
	std::vector< Ptr<Pin> >& pinvector = (direction == Pin::in) ? m_inpins : m_outpins;
	Pin::Side side = (direction == Pin::in) ? Pin::left : Pin::right;

	size_t npins = pins.size();
	float posstep = 1.0f / (float)(npins + 1);
	float pos = posstep;

	for (std::vector<InfoImpl::PinInfo*>::const_iterator i = pins.begin(); i != pins.end(); ++i)
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
