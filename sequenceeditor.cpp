#include "stdafx.h"
#include "common.h"
#include "sequenceeditor.h"
using namespace SequenceEditor;

/* TRANSLATOR SequenceEditor::Editor */

PrefsVar_Int Editor::s_prefTrackHeaderWidth("sequenceeditor/headerwidth", 200);
PrefsVar_Int Editor::s_prefRulerHeight("sequenceeditor/rulerheight", 50);

enum
{
	zZero = 0,
	zTrack,
	zSelection,
};

Editor::Editor(const Ptr<Sequence>& seq, QWidget* parent)
: m_seq(seq), MyGraphicsView(parent), m_scene(this)
{
	setScene(&m_scene);
	setAlignment(Qt::AlignTop | Qt::AlignLeft);

	m_scene.setSceneRect(0, 0, getSceneWidth(), 1);
	scale(100, 1);

	setViewportMargins(s_prefTrackHeaderWidth(), s_prefRulerHeight(), 0, 0);

	m_trackHeaderScrollArea = new QScrollArea(this);
	m_trackHeaderScrollArea->move(0, s_prefRulerHeight()+1);
	m_trackHeaderScrollArea->resize(s_prefTrackHeaderWidth(), 500);
	m_trackHeaderScrollArea->setFrameStyle(QFrame::NoFrame);

	m_trackHeaderParent = new TrackHeaderParent;
	m_trackHeaderParent->resize(s_prefTrackHeaderWidth(), 500);
	m_trackHeaderLayout = new QVBoxLayout(m_trackHeaderParent);
	m_trackHeaderLayout->setSpacing(0);
	m_trackHeaderLayout->setContentsMargins(0,0,0,0);

	m_trackHeaderScrollArea->setWidget(m_trackHeaderParent);
	m_trackHeaderScrollArea->setWidgetResizable(true);
	m_trackHeaderScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_trackHeaderScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	connect( m_trackHeaderParent, SIGNAL(signalHeightChanged(int)),
		&m_scene, SLOT(setHeight(int)) );

	// Tie the vertical scrolling of the main view to that of the track headers
	connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
		m_trackHeaderScrollArea->verticalScrollBar(), SLOT(setValue(int)) );
	connect(m_trackHeaderScrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)),
		verticalScrollBar(), SLOT(setValue(int)) );

	m_trackHeaderLayout->addStretch(1);

	for (int i=0; i<m_seq->getNumTracks(); i++)
	{
		onTrackAdded(i, m_seq->getTrack(i));
	}

	connect( m_seq, SIGNAL(signalAddTrack(int, const Ptr<Seq::Track>&)),
		this, SLOT(onTrackAdded(int, const Ptr<Seq::Track>&)) );
	connect( m_seq, SIGNAL(signalRemoveTrack(int, const Ptr<Seq::Track>&)),
		this, SLOT(onTrackRemoved(int, const Ptr<Seq::Track>&)) );

	m_itemSelection = new QGraphicsRectItem;
	
	QColor c(Qt::black);
	m_itemSelection->setPen(c);

	c = Qt::white;
	c.setAlphaF(0.5);
	m_itemSelection->setBrush(c);

	m_itemSelection->setZValue(zSelection);
	m_scene.addItem(m_itemSelection);
}

void Editor::resizeEvent(QResizeEvent* ev)
{
	QGraphicsView::resizeEvent(ev);
	m_trackHeaderScrollArea->resize(s_prefTrackHeaderWidth(), ev->size().height());
}

void Editor::onTrackAdded(int index, const Ptr<Seq::Track>& track)
{
	TrackHeader* header = new TrackHeader(this, track);
	m_trackHeaders.insert(track, header);
	m_trackHeaderLayout->insertWidget(index, header);

	Track* item = new Track(this, track);
	m_tracks.insert(track, item);
	m_scene.addItem(item);

	item->setHeight(header->height());
	connect( header, SIGNAL(signalHeightChanged(int)),
		item, SLOT(setHeight(int)) );

	item->setY(header->y());
	connect( header, SIGNAL(signalYChanged(int)),
		item, SLOT(setY(int)) );
}

void Editor::onTrackRemoved(int index, const Ptr<Seq::Track>& track)
{
	TrackHeader* header = m_trackHeaders.take(track);
	if (header)
	{
		m_trackHeaderLayout->removeWidget(header);
		delete header;
	}

	Track* item = m_tracks.take(track);
	if (item)
	{
		delete item;
	}
}

void Editor::updateSelectionItem()
{
	QRectF rect;
	rect.setLeft ((double)m_selStart / c_pow_2_32);
	rect.setRight((double)m_selEnd   / c_pow_2_32);

	TrackHeader* header = m_trackHeaders.value(m_seq->getTrack(m_selFirstTrack));
	rect.setTop(header->y());

	if (m_selLastTrack != m_selFirstTrack)
		header = m_trackHeaders.value(m_seq->getTrack(m_selLastTrack));
	rect.setBottom(header->y() + header->height());

	m_itemSelection->setRect(rect);
}

////////////////////////////////////////////////////////////////////////////////////

TrackHeader::TrackHeader(Editor* editor, const Ptr<Seq::Track>& track)
: m_editor(editor), m_track(track)
{
	ui.setupUi(this);

	ui.labelMachineName->setText(m_track->getMachine()->getName());

	forceHeight(100);

	connect( m_track->getMachine(), SIGNAL(signalRename(const QString&)),
		ui.labelMachineName, SLOT(setText(const QString&)) );
}

void TrackHeader::resizeEvent(QResizeEvent* ev)
{
	QWidget::resizeEvent(ev);
	signalHeightChanged(ev->size().height());
}

void TrackHeader::moveEvent(QMoveEvent* ev)
{
	QWidget::moveEvent(ev);
	signalYChanged(ev->pos().y());
}

////////////////////////////////////////////////////////////////////////////////////

Track::Track(Editor* editor, const Ptr<Seq::Track>& track)
: m_editor(editor), m_track(track)
{
	setZValue(zTrack);
	setPen(Qt::NoPen);
	setBrush(QColor("light steel blue"));
}

void Track::setHeight(int height)
{
	resetTransform();
	scale(1, height-1);
	setRect(0, 0, m_editor->getSceneWidth(), 1);
}

void Track::setY(int y)
{
	setPos(0, y);
}

/////////////////////////////////////////////////////////////////////////////////

void Track::mousePressEvent(QGraphicsSceneMouseEvent* ev)
{
	m_editor->startSelectionDrag(m_track, ev);
}

void Track::mouseMoveEvent(QGraphicsSceneMouseEvent* ev)
{
	m_editor->continueSelectionDrag(ev);
}

void Track::mouseReleaseEvent(QGraphicsSceneMouseEvent* ev)
{
	m_editor->endSelectionDrag(ev);
}

void Editor::startSelectionDrag(const Ptr<Seq::Track>& track, QGraphicsSceneMouseEvent* ev)
{
	QPointF pos = ev->scenePos();

	m_selDragStart = m_seq->getSnapPoint((int64)(pos.x() * c_pow_2_32));
	m_selDragStart = max(0LL, m_selDragStart);
	m_selDragStartTrack = m_seq->getTrackIndex(track);
	
	continueSelectionDrag(ev);
}

void Editor::continueSelectionDrag(QGraphicsSceneMouseEvent* ev)
{
	QPointF pos = ev->scenePos();

	int64 end = m_seq->getSnapPoint((int64)(pos.x() * c_pow_2_32));
	end = max(0LL, end);

	m_selStart = min(m_selDragStart, end);
	m_selEnd   = max(m_selDragStart, end);

	Track* track = getTrackAtY(pos.y());
	if (track)
	{
		int index = m_seq->getTrackIndex(track->getTrack());

		m_selFirstTrack = min(m_selDragStartTrack, index);
		m_selLastTrack  = max(m_selDragStartTrack, index);
	}

	updateSelectionItem();
}

void Editor::endSelectionDrag(QGraphicsSceneMouseEvent* ev)
{
	continueSelectionDrag(ev);
}

Track* Editor::getTrackAtY(double y)
{
	QPointF p(getSceneWidth()*0.5, y);
	foreach(QGraphicsItem* item, m_scene.items(p))
	{
		Track* track = dynamic_cast<Track*>(item);
		if (track) return track;
	}
	return NULL;
}

////////////////////////////////////////////////////////////////////////////////////

Scene::Scene(Editor* ed)
: m_editor(ed)
{
}

void Scene::setHeight(int h)
{
	QRectF rect = sceneRect();
	rect.setHeight(h);
	setSceneRect(rect);
}

bool Scene::shouldAcceptDropEvent(QGraphicsSceneDragDropEvent* ev)
{
	return false;
}

void Scene::dragEnterEvent(QGraphicsSceneDragDropEvent* ev)
{
	if (shouldAcceptDropEvent(ev))
		ev->acceptProposedAction();
	else
		ev->ignore();
}

void Scene::dragMoveEvent(QGraphicsSceneDragDropEvent* ev)
{
	if (shouldAcceptDropEvent(ev))
		ev->acceptProposedAction();
	else
		ev->ignore();
}

void Scene::dropEvent(QGraphicsSceneDragDropEvent* ev)
{
	if (shouldAcceptDropEvent(ev))
	{
		ev->acceptProposedAction();
	}
	else
	{
		ev->ignore();
	}
}
