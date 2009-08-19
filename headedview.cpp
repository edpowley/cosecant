#include "stdafx.h"
#include "common.h"
#include "headedview.h"

HeadedView::HeadedView(QWidget* parent)
: QSplitter(Qt::Horizontal, parent), m_firstShow(true)
{
	QWidget* whead = new QWidget(this);
	QWidget* wbody = new QWidget(this);
	addWidget(whead); addWidget(wbody);
	setStretchFactor(0,0); setStretchFactor(1,1);

	m_headView = new MyGraphicsView(this);
	m_rulerView = new MyGraphicsView(this);
	m_bodyView = new MyGraphicsView(this);

	m_headView->setFocusPolicy(Qt::NoFocus);
	m_rulerView->setFocusPolicy(Qt::NoFocus);

	m_lhead = new QVBoxLayout(whead);
	m_lhead->setContentsMargins(0,0,0,0);
	m_lhead->addSpacing(c_rulerHeight + m_lhead->spacing());
	m_lhead->addWidget(m_headView, 1);
	
	m_lruler = new QHBoxLayout;
	m_lruler->setContentsMargins(0,0,0,0);
	m_lruler->addWidget(m_rulerView, 1);

	m_lbody = new QVBoxLayout(wbody);
	m_lbody->setContentsMargins(0,0,0,0);
	m_rulerView->setMinimumHeight(c_rulerHeight);
	m_rulerView->setMaximumHeight(c_rulerHeight);
	m_lbody->addLayout(m_lruler, 0);
	m_lbody->addWidget(m_bodyView, 1);

	m_headView ->setHorizontalScrollBarPolicy	(Qt::ScrollBarAlwaysOff);
	m_headView ->setVerticalScrollBarPolicy		(Qt::ScrollBarAlwaysOff);
	m_rulerView->setHorizontalScrollBarPolicy	(Qt::ScrollBarAlwaysOff);
	m_rulerView->setVerticalScrollBarPolicy		(Qt::ScrollBarAlwaysOff);
	m_bodyView ->setHorizontalScrollBarPolicy	(Qt::ScrollBarAlwaysOn);
	m_bodyView ->setVerticalScrollBarPolicy		(Qt::ScrollBarAlwaysOn);

	connect(
		m_bodyView->horizontalScrollBar(), SIGNAL(valueChanged(int)),
		m_rulerView->horizontalScrollBar(), SLOT(setValue(int)) );
	connect(
		m_rulerView->horizontalScrollBar(), SIGNAL(valueChanged(int)),
		m_bodyView->horizontalScrollBar(), SLOT(setValue(int)) );

	connect(
		m_bodyView->verticalScrollBar(), SIGNAL(valueChanged(int)),
		m_headView->verticalScrollBar(), SLOT(setValue(int)) );
	connect(
		m_headView->verticalScrollBar(), SIGNAL(valueChanged(int)),
		m_bodyView->verticalScrollBar(), SLOT(setValue(int)) );

	m_rulerView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	m_headView ->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	m_bodyView ->setAlignment(Qt::AlignLeft | Qt::AlignTop);
}

void HeadedView::showEvent(QShowEvent* ev)
{
	if (m_firstShow)
	{
		m_lhead->addSpacing(m_bodyView->horizontalScrollBar()->height());
		m_lruler->addSpacing(m_bodyView->verticalScrollBar()->width());
	}

	m_firstShow = false;

	QSplitter::showEvent(ev);
}
