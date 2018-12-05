#include "stdafx.h"
#include <QGroupBox>
#include <QWidget>
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

	m_showTimeLine = new QTimeLine(500, this);
	connect(
		m_showTimeLine, SIGNAL(frameChanged(int)),
		this, SLOT(setContentHeight(int)) );
	connect(
		m_showTimeLine, SIGNAL(finished()),
		this, SLOT(unsetContentHeight()) );

	m_hideTimeLine = new QTimeLine(500, this);
	connect(
		m_hideTimeLine, SIGNAL(frameChanged(int)),
		this, SLOT(setContentHeight(int)) );

	connect(
		this, SIGNAL(toggled(bool)),
		this, SLOT(onToggled(bool)) );
}

void ExpanderBox::onToggled(bool toggle)
{
	if (toggle)
	{
		m_showTimeLine->setFrameRange(0, m_content->sizeHint().height());
		m_showTimeLine->start();
	}
	else
	{
		m_hideTimeLine->setFrameRange(m_content->height(), 0);
		m_hideTimeLine->start();
	}
}

void ExpanderBox::setContentHeight(int h)
{
	m_content->setMaximumHeight(h);
}

void ExpanderBox::unsetContentHeight()
{
	m_content->setMaximumHeight(QWIDGETSIZE_MAX);
}
