#include "stdafx.h"
#include "common.h"
#include "parameditor.h"
#include "machine.h"
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

int Parameter::Real::addToParamEditor(QGridLayout* grid, int row)
{
	grid->addWidget(new QLabel(m_name), row, 0);

	ScalarSlider* slider = new ScalarSlider(this);
	grid->addWidget(slider, row, 1);

	ScalarEdit* edit = new ScalarEdit(this);
	grid->addWidget(edit, row, 2);

	return row+1;
}

ScalarSlider::ScalarSlider(const Ptr<Parameter::Scalar>& param)
: m_param(param), QSlider(Qt::Horizontal), m_valueChanging(false)
{
	setRange(0, c_intRangeMax);
	setValue(valueToInt(m_param->getState()));

	connect(
		this, SIGNAL(valueChanged(int)),
		this, SLOT(onValueChanged(int)) );
	connect(
		m_param, SIGNAL(valueChanged(double)),
		this, SLOT(onParameterChanged(double)) );
}

int ScalarSlider::valueToInt(double value)
{
	return remap(value, m_param->getMin(), m_param->getMax(), 0, c_intRangeMax);
}

void ScalarSlider::onValueChanged(int value)
{
	m_valueChanging = true;
	double pval = remap(value, 0, c_intRangeMax, m_param->getMin(), m_param->getMax());
	m_param->change(pval);
	m_valueChanging = false;
}

void ScalarSlider::onParameterChanged(double value)
{
	setValue(valueToInt(value));
}

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
		m_param->change(v);
	}
}

void ScalarEdit::focusOutEvent(QFocusEvent* ev)
{
	QLineEdit::focusOutEvent(ev);

	setTextFromValue(m_param->getState());
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
: m_param(param)
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
	setCurrentIndex((int)value);
}

void EnumCombo::onCurrentIndexChanged(int index)
{
	m_param->change(index);
}
