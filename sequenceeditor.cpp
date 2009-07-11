#include "stdafx.h"
#include "common.h"
#include "sequenceeditor.h"
using namespace SequenceEditor;

/* TRANSLATOR SequenceEditor::Editor */

Editor::Editor(const Ptr<Sequence::Seq>& seq, QWidget* parent)
: QSplitter(parent), m_seq(seq), m_pixelsPerSecond(16)
{
	QWidget* whead = new QWidget(this);
	QWidget* wbody = new QWidget(this);
	addWidget(whead); addWidget(wbody);
	setStretchFactor(0,0); setStretchFactor(1,1);

	int rulersize = 50;

	m_headView = new MyGraphicsView(this);
	m_rulerView = new MyGraphicsView(this);
	m_bodyView = new MyGraphicsView(this);

	QVBoxLayout* lhead = new QVBoxLayout(whead);
	lhead->setContentsMargins(0,0,0,0);
	lhead->addSpacing(rulersize + lhead->spacing());
	lhead->addWidget(m_headView, 1);
	
	QVBoxLayout* lbody = new QVBoxLayout(wbody);
	lbody->setContentsMargins(0,0,0,0);
	m_rulerView->setMinimumHeight(rulersize);
	m_rulerView->setMaximumHeight(rulersize);
	lbody->addWidget(m_rulerView, 0);
	lbody->addWidget(m_bodyView, 1);

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
		m_bodyView->verticalScrollBar(), SIGNAL(valueChanged(int)),
		m_headView->verticalScrollBar(), SLOT(setValue(int)) );

	m_rulerView->setScene(&m_rulerScene);
	m_headView ->setScene(&m_headScene);
	m_bodyView ->setScene(&m_bodyScene);
	m_rulerView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	m_headView ->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	m_bodyView ->setAlignment(Qt::AlignLeft | Qt::AlignTop);

	createTrackItems();
}

qreal Editor::getBodyWidth()
{
	return m_pixelsPerSecond * m_seq->getLengthInSeconds();
}

void Editor::createTrackItems()
{
	qreal y = 0.0;
	BOOST_FOREACH(const Ptr<Sequence::Track>& track, m_seq->m_tracks)
	{
		TrackItem* item = new TrackItem(this, track);
		m_bodyScene.addItem(item);
		item->setPos(0, y);
		y += item->rect().height();
	}
}

///////////////////////////////////////////////////////////////////////////////

TrackItem::TrackItem(Editor* editor, const Ptr<Sequence::Track>& track)
: m_editor(editor), m_track(track), QGraphicsRectItem(0, 0, editor->getBodyWidth(), track->getHeight())
{
	setPen(Qt::NoPen);
	QLinearGradient grad(0, 0, 0, rect().height());
	grad.setColorAt(0, Qt::white);
	grad.setColorAt(1, Qt::gray);
	setBrush(grad);
}
