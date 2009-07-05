#include "stdafx.h"
#include "common.h"
#include "expanderbox.h"

ExpanderBox::ExpanderBox(const QString& title, QWidget* parent)
: QGroupBox(title, parent)
{
	setCheckable(true);

	m_content = new QWidget(this);
	QVBoxLayout* innerLayout = new QVBoxLayout(this);
	innerLayout->setContentsMargins(0,0,0,0);
	innerLayout->addWidget(m_content);

	connect(
		this, SIGNAL(toggled(bool)),
		m_content, SLOT(setVisible(bool)) );
}

