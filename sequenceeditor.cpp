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

	m_trackHeaderParent = new QSplitter(Qt::Vertical);
	m_trackHeaderParent->resize(s_prefTrackHeaderWidth(), 1000);

	m_trackHeaderScrollArea->setWidget(m_trackHeaderParent);
	m_trackHeaderScrollArea->setWidgetResizable(false);
	m_trackHeaderScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	m_trackHeaderScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

	// Tie the vertical scrolling of the main view to that of the track headers
	connect(verticalScrollBar(), SIGNAL(valueChanged(int)),
		m_trackHeaderScrollArea->verticalScrollBar(), SLOT(setValue(int)) );
	connect(m_trackHeaderScrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)),
		verticalScrollBar(), SLOT(setValue(int)) );

	for (int i=0; i<10; i++)
	{
		m_trackHeaderParent->addWidget(new TrackHeader);
	}
}

void Editor::resizeEvent(QResizeEvent* ev)
{
	m_trackHeaderScrollArea->resize(s_prefTrackHeaderWidth(), ev->size().height());
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
