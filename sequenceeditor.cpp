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

	connect(
		m_seq, SIGNAL(signalInsertTrack(int, const Ptr<Sequence::Track>&)),
		this, SLOT(onInsertTrack(int, const Ptr<Sequence::Track>&)) );
	connect(
		m_seq, SIGNAL(signalRemoveTrack(int, const Ptr<Sequence::Track>&)),
		this, SLOT(onRemoveTrack(int, const Ptr<Sequence::Track>&)) );
}

qreal Editor::getBodyWidth()
{
	return m_pixelsPerSecond * m_seq->getLengthInSeconds();
}

void Editor::createTrackItems()
{
	qreal y = 0.0;
	for (int i=0; i<m_seq->m_tracks.length(); ++i)
	{
		TrackItem* item = new TrackItem(this, m_seq->m_tracks[i]);
		m_bodyScene.addItem(item);
		item->setPos(0, y);

		m_trackItems.push_back(item);

		y += item->height();
	}
}

void Editor::onInsertTrack(int index, const Ptr<Sequence::Track>& track)
{
	qreal y = 0.0;
	if (index < m_trackItems.length())
		y = m_trackItems[index]->y();
	else if (index > 0)
		y = m_trackItems[index-1]->y() + m_trackItems[index-1]->height();

	TrackItem* item = new TrackItem(this, track);
	m_bodyScene.addItem(item);
	item->setPos(0, y);

	m_trackItems.insert(index, item);

	// Move the subsequent items down
	qreal h = item->height();
	for (int i = index+1; i < m_trackItems.length(); ++i)
	{
		m_trackItems[i]->moveBy(0, h);
	}
}

void Editor::onRemoveTrack(int index, const Ptr<Sequence::Track>& track)
{
	TrackItem* item = m_trackItems.takeAt(index); // removes it from the list
	m_bodyScene.removeItem(item);

	// Move the subsequent items up
	qreal h = item->height();
	for (int i = index; i < m_trackItems.length(); ++i)
	{
		m_trackItems[i]->moveBy(0, -h);
	}

	delete item;
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
