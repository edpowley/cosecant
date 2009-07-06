#include "stdafx.h"
#include "common.h"
#include "parameditor.h"
#include "machine.h"
#include "song.h"
#include "expanderbox.h"

using namespace ParamEditorWidget;

ParamEditor::ParamEditor(const Ptr<Machine>& mac, QWidget* parent)
: QScrollArea(parent), m_mac(mac)
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
}

ScalarChangeCommand::ScalarChangeCommand(const Ptr<Parameter::Scalar>& param, double newval, bool mergeable)
: m_param(param), m_oldValue(param->getState()), m_newValue(newval), m_mergeable(mergeable)
{
	setText(ParamEditor::tr("change parameter '%1' on '%2'")
		.arg(param->getName()).arg(param->getMachine()->m_name)
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

	connect(
		m_param, SIGNAL(valueChanged(double)),
		this, SLOT(onParameterChanged(double)) );
	connect(
		this, SIGNAL(returnPressed()),
		this, SLOT(onReturnPressed()) );
}

void ScalarEdit::setTextFromValue(double value)
{
	setText(m_locale.toString(value, 'g', 5));
}

void ScalarEdit::onParameterChanged(double value)
{
	setTextFromValue(value);
}

void ScalarEdit::onReturnPressed()
{
	bool ok = false;
	double v = text().toDouble(&ok);
	if (ok)
	{
		theUndo().push(new ScalarChangeCommand(m_param, v, false));
	}
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
	setRange(m_param->getMin(), m_param->getMax());
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

//	ScalarSlider* slider = new ScalarSlider(this);
//	grid->addWidget(slider, row, 1);

	return row+1;
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
