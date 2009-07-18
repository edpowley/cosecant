#include "stdafx.h"
#include "common.h"
#include "parameditor.h"
#include "machine.h"
#include "song.h"
#include "expanderbox.h"
#include "timeunit_convert.h"

using namespace ParamEditorWidget;

ParamEditor::ParamEditor(const Ptr<Machine>& mac, QDockWidget* parent)
: QScrollArea(parent), m_mac(mac), m_parent(parent)
{
	QWidget* w = new QWidget(this);
	QGridLayout* grid = new QGridLayout(w);
	m_mac->m_params->populateParamEditorGrid(grid);

	// Add a spacer, so that the table doesn't try to extend vertically to fill the window
	int lastrow = grid->rowCount();
	grid->addItem(new QSpacerItem(1,1), lastrow, 0, 1, -1);
	grid->setRowStretch(lastrow, 1);

	setWidget(w);
	setWidgetResizable(true);

	if (!m_mac->m_parameditor)
		m_mac->m_parameditor = this;

	updateWindowTitle(m_mac->getName());

	connect(
		m_mac, SIGNAL(signalRename(const QString&)),
		this, SLOT(updateWindowTitle(const QString&)) );
}

ParamEditor::~ParamEditor()
{
	if (m_mac->m_parameditor == this)
		m_mac->m_parameditor = NULL;
}

void ParamEditor::updateWindowTitle(const QString& macname)
{
	m_parent->setWindowTitle(tr("Parameters - %1").arg(macname));
}

ScalarChangeCommand::ScalarChangeCommand(const Ptr<Parameter::Scalar>& param, double newval, bool mergeable)
: m_param(param), m_oldValue(param->getState()), m_newValue(newval), m_mergeable(mergeable)
{
	setText(ParamEditor::tr("change parameter '%1' on '%2'")
		.arg(param->getName()).arg(param->getMachine()->getName())
	);
}

void ScalarChangeCommand::redo() { m_param->change(m_newValue); }
void ScalarChangeCommand::undo() { m_param->change(m_oldValue); }

bool ScalarChangeCommand::mergeWith(const QUndoCommand* qcommand)
{
	if (!m_mergeable) return false;
	const ScalarChangeCommand* cmd = dynamic_cast<const ScalarChangeCommand*>(qcommand);
	if (cmd && cmd->m_param == m_param)
	{
		if (!cmd->m_mergeable) return false;
		m_newValue = cmd->m_newValue;
		return true;
	}
	else
		return false;
}

////////////////////////////////////////////////////////////////////////////////////

void Parameter::Group::populateParamEditorGrid(QGridLayout* grid)
{
	grid->setColumnStretch(1, 100);
	grid->setColumnMinimumWidth(1, 50);
	grid->setColumnStretch(2, 1);
	int row = 0;
	BOOST_FOREACH(const Ptr<Parameter::Base>& param, m_params)
	{
		row = param->addToParamEditor(grid, row);
	}
}

int Parameter::Group::addToParamEditor(QGridLayout* grid, int row)
{
	ExpanderBox* box = new ExpanderBox(m_name);
	grid->addWidget(box, row, 0, 1, -1);

	QGridLayout* innerGrid = new QGridLayout(box->getContent());

	populateParamEditorGrid(innerGrid);

	return row+1;
}

///////////////////////////////////////////////////////////////////////////////////

ScalarSlider::ScalarSlider(const Ptr<Parameter::Scalar>& param)
: m_param(param), QSlider(Qt::Horizontal), m_valueChanging(false)
{
	m_logarithmic = m_param->hasFlag(ParamFlags::logarithmic);

	connect(
		this, SIGNAL(valueChanged(int)),
		this, SLOT(onValueChanged(int)) );
	connect(
		m_param, SIGNAL(valueChanged(double)),
		this, SLOT(onParameterChanged(double)) );
}

void ScalarSlider::initFromState()
{
	m_valueChanging = true;
	setValue(valueToInt(m_param->getState()));
	m_valueChanging = false;
}

void ScalarSlider::onValueChanged(int value)
{
	if (!m_valueChanging)
	{
		double pval = intToValue(value);
		theUndo().push(new ScalarChangeCommand(m_param, pval, true));
	}
}

void ScalarSlider::onParameterChanged(double value)
{
	m_valueChanging = true;
	setValue(valueToInt(value));
	m_valueChanging = false;
}

///////////////////////////////////////////////////////////////////////////////////

ScalarEdit::ScalarEdit(const Ptr<Parameter::Scalar>& param)
: m_param(param)
{
	m_locale.setNumberOptions(QLocale::OmitGroupSeparator);
	setValidator(new QDoubleValidator(this));
	setTextFromValue(m_param->getState());
	setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

	connect(
		m_param, SIGNAL(valueChanged(double)),
		this, SLOT(onParameterChanged(double)) );
	connect(
		this, SIGNAL(returnPressed()),
		this, SLOT(onReturnPressed()) );
}

void ScalarEdit::setTextFromValue(double value)
{
	setText(m_locale.toString(value, 'g', -1));
	setCursorPosition(0);
}

double ScalarEdit::getValueFromText()
{
	return text().toDouble();
}

void ScalarEdit::onParameterChanged(double value)
{
	setTextFromValue(value);
}

void ScalarEdit::onReturnPressed()
{
	double v = getValueFromText();
	theUndo().push(new ScalarChangeCommand(m_param, v, false));
}

void ScalarEdit::focusOutEvent(QFocusEvent* ev)
{
	QLineEdit::focusOutEvent(ev);

	setTextFromValue(m_param->getState());
}

////////////////////////////////////////////////////////////////////////////////////

int Parameter::Real::addToParamEditor(QGridLayout* grid, int row)
{
	grid->addWidget(new QLabel(m_name), row, 0);

	RealSlider* slider = new RealSlider(this);
	slider->initFromState();
	grid->addWidget(slider, row, 1);

	ScalarEdit* edit = new ScalarEdit(this);
	grid->addWidget(edit, row, 2);

	return row+1;
}

RealSlider::RealSlider(const Ptr<Parameter::Scalar>& param)
: ScalarSlider(param)
{
	setRange(0, c_intRangeMax);
}

int RealSlider::valueToInt(double value)
{
	if (m_logarithmic)
	{
		return remap( log(value), log(m_param->getMin()), log(m_param->getMax()), 0, c_intRangeMax );
	}
	else
	{
		return remap(value, m_param->getMin(), m_param->getMax(), 0, c_intRangeMax);
	}
}

double RealSlider::intToValue(int i)
{
	double v;
	if (m_logarithmic)
	{
		v = exp(remap( i, 0, c_intRangeMax, log(m_param->getMin()), log(m_param->getMax()) ));
	}
	else
	{
		v = remap(i, 0, c_intRangeMax, m_param->getMin(), m_param->getMax());
	}

	return v;
}

///////////////////////////////////////////////////////////////////////////////////

int Parameter::Int::addToParamEditor(QGridLayout* grid, int row)
{
	grid->addWidget(new QLabel(m_name), row, 0);

	IntSlider* slider = new IntSlider(this);
	slider->initFromState();
	grid->addWidget(slider, row, 1);

	ScalarEdit* edit = new ScalarEdit(this);
	grid->addWidget(edit, row, 2);

	return row+1;
}

IntSlider::IntSlider(const Ptr<Parameter::Int>& param)
: ScalarSlider(param), m_param(param)
{
	m_valueChanging = true;
	setRange(m_param->getMin(), m_param->getMax());
	m_valueChanging = false;
}

int IntSlider::valueToInt(double value)
{
	return (int)floor(value + 0.5);
}

double IntSlider::intToValue(int i)
{
	return i;
}

////////////////////////////////////////////////////////////////////////////////////

int Parameter::Time::addToParamEditor(QGridLayout* grid, int row)
{
	grid->addWidget(new QLabel(m_name), row, 0);

	TimeSlider* slider = new TimeSlider(this);
	slider->initFromState();
	grid->addWidget(slider, row, 1);

	TimeEdit* edit = new TimeEdit(this);
	grid->addWidget(edit, row, 2);

	TimeUnitCombo* unitcombo = new TimeUnitCombo(this);
	grid->addWidget(unitcombo, row, 3);

	connect(
		unitcombo, SIGNAL(signalUnitChanged(TimeUnit::unit)),
		slider, SLOT(setUnit(TimeUnit::unit)) );
	connect(
		unitcombo, SIGNAL(signalUnitChanged(TimeUnit::unit)),
		edit, SLOT(setUnit(TimeUnit::unit)) );

	return row+1;
}

TimeSlider::TimeSlider(const Ptr<Parameter::Time>& param)
: RealSlider(param), m_param(param)
{
	setUnit(m_param->getDisplayUnit());
}

void TimeSlider::setUnit(TimeUnit::unit unit)
{
	m_unit = unit;
	m_min = ConvertTimeUnits(m_param->getTMin(), m_unit);
	m_max = ConvertTimeUnits(m_param->getTMax(), m_unit);
	if (m_min > m_max) std::swap(m_min, m_max);
	m_logarithmic = (m_unit == TimeUnit::hertz);
	
	initFromState();
}

int TimeSlider::valueToInt(double value)
{
	value = ConvertTimeUnits(m_param->getInternalUnit(), m_unit, value);

	if (m_logarithmic)
	{
		return remap( log(value), log(m_min), log(m_max), 0, c_intRangeMax );
	}
	else
	{
		return remap(value, m_min, m_max, 0, c_intRangeMax);
	}
}

double TimeSlider::intToValue(int i)
{
	double v;
	if (m_logarithmic)
	{
		v = exp(remap( i, 0, c_intRangeMax, log(m_min), log(m_max) ));
	}
	else
	{
		v = remap(i, 0, c_intRangeMax, m_min, m_max);
	}

	return ConvertTimeUnits(m_unit, m_param->getInternalUnit(), v);
}

TimeEdit::TimeEdit(const Ptr<Parameter::Time>& param)
: ScalarEdit(param), m_param(param)
{
	setUnit(m_param->getDisplayUnit());
}

void TimeEdit::setUnit(TimeUnit::unit unit)
{
	m_unit = unit;
	setTextFromValue(m_param->getState());
}

void TimeEdit::setTextFromValue(double value)
{
	ScalarEdit::setTextFromValue(ConvertTimeUnits(m_param->getInternalUnit(), m_unit, value));
}

double TimeEdit::getValueFromText()
{
	return ConvertTimeUnits(m_unit, m_param->getInternalUnit(), ScalarEdit::getValueFromText());
}

TimeUnitCombo::TimeUnitCombo(const Ptr<Parameter::Time>& param)
: m_param(param)
{
	unsigned int units = m_param->getDisplayUnits();

	for (unsigned int i = 0; i < TimeUnit::numUnits; ++i)
	{
		if (units & (1 << i))
		{
			addItem(getUnitName(1 << i), QVariant::fromValue<unsigned int>(1 << i));
		}
	}

	int index = findUnit(m_param->getDisplayUnit());
	if (index != -1)
		setCurrentIndex(index);
	else
		qDebug() << "Warning: failed to find display unit in TimeUnitCombo";

	connect(
		this, SIGNAL(currentIndexChanged(int)),
		this, SLOT(onCurrentIndexChanged(int)) );
}

int TimeUnitCombo::findUnit(TimeUnit::unit unit)
{
	return findData( QVariant::fromValue<unsigned int>(unit) );
}

QString TimeUnitCombo::getUnitName(TimeUnit::unit unit)
{
	switch (unit)
	{
	case TimeUnit::seconds:		return tr("seconds");
	case TimeUnit::beats:		return tr("beats");
	case TimeUnit::samples:		return tr("samples");
	case TimeUnit::hertz:		return tr("Hz");
	case TimeUnit::fracfreq:	return tr("Fs");
	case TimeUnit::notenum:		return tr("note");
	default:					return "???";
	}
}

void TimeUnitCombo::onCurrentIndexChanged(int index)
{
//	if (!m_valueChanging)
	{
		TimeUnit::unit unit = static_cast<TimeUnit::unit>( itemData(index).value<unsigned int>() );
		m_param->setDisplayUnit(unit);
		signalUnitChanged(unit);
	}
}

//////////////////////////////////////////////////////////////////////////////////////

int Parameter::Enum::addToParamEditor(QGridLayout* grid, int row)
{
	grid->addWidget(new QLabel(m_name), row, 0);

	EnumCombo* combo = new EnumCombo(this);
	grid->addWidget(combo, row, 1, 1, 2);

	return row+1;
}

EnumCombo::EnumCombo(const Ptr<Parameter::Enum>& param)
: m_param(param), m_valueChanging(false)
{
	setSizeAdjustPolicy(AdjustToMinimumContentsLength);
	addItems(m_param->getItems());
	setCurrentIndex((int)m_param->getState());

	connect(
		m_param, SIGNAL(valueChanged(double)),
		this, SLOT(onParameterChanged(double)) );
	connect(
		this, SIGNAL(currentIndexChanged(int)),
		this, SLOT(onCurrentIndexChanged(int)) );
}

void EnumCombo::onParameterChanged(double value)
{
	m_valueChanging = true;
	setCurrentIndex((int)value);
	m_valueChanging = false;
}

void EnumCombo::onCurrentIndexChanged(int index)
{
	if (!m_valueChanging) theUndo().push(new ScalarChangeCommand(m_param, index, false));
}
