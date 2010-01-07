#include "stdafx.h"
#include "common.h"
#include "sequenceeditor.h"
using namespace SequenceEditor;

/* TRANSLATOR SequenceEditor::Editor */

PrefsVar_Int Editor::s_prefTrackHeaderWidth("sequenceeditor/headerwidth", 200);
PrefsVar_Int Editor::s_prefRulerHeight("sequenceeditor/rulerheight", 50);

Editor::Editor(const Ptr<Sequence>& seq, QWidget* parent)
: m_seq(seq), MyGraphicsView(parent), m_scene(this)
{
	setScene(&m_scene);
	setDragMode(RubberBandDrag);

	m_scene.setSceneRect(0,0,1000,1000);

	setViewportMargins(s_prefTrackHeaderWidth(), s_prefRulerHeight(), 0, 0);

	m_trackHeaderScrollArea = new QScrollArea(this);
	m_trackHeaderScrollArea->move(0, s_prefRulerHeight());
	m_trackHeaderScrollArea->resize(s_prefTrackHeaderWidth(), 500);
	m_trackHeaderScrollArea->setFrameStyle(QFrame::NoFrame);

	m_trackHeaderParent = new QWidget;
	m_trackHeaderParent->resize(s_prefTrackHeaderWidth(), 1000);
	m_trackHeaderLayout = new QVBoxLayout(m_trackHeaderParent);
	m_trackHeaderLayout->setSpacing(0);
	m_trackHeaderLayout->setContentsMargins(0,0,0,0);

	m_trackHeaderScrollArea->setWidget(m_trackHeaderParent);
	m_trackHeaderScrollArea->setWidgetResizable(false);
	m_trackHeaderScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_trackHeaderScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

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

/*	for (int i=0; i<10; i++)
	{
		m_trackHeaderLayout->addWidget(new TrackHeader);
	}
*/

	connect( m_seq, SIGNAL(signalAddTrack(int, const Ptr<Seq::Track>&)),
		this, SLOT(onTrackAdded(int, const Ptr<Seq::Track>&)) );
	connect( m_seq, SIGNAL(signalRemoveTrack(int, const Ptr<Seq::Track>&)),
		this, SLOT(onTrackRemoved(int, const Ptr<Seq::Track>&)) );
}

void Editor::resizeEvent(QResizeEvent* ev)
{
	m_trackHeaderScrollArea->resize(s_prefTrackHeaderWidth(), ev->size().height());
}

void Editor::onTrackAdded(int index, const Ptr<Seq::Track>& track)
{
	TrackHeader* header = new TrackHeader(this, track);
	m_trackHeaders.insert(track, header);
	m_trackHeaderLayout->insertWidget(index, header);
}

void Editor::onTrackRemoved(int index, const Ptr<Seq::Track>& track)
{
	TrackHeader* header = m_trackHeaders.take(track);
	if (header)
	{
		m_trackHeaderLayout->removeWidget(header);
		delete header;
	}
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

////////////////////////////////////////////////////////////////////////////////////

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
