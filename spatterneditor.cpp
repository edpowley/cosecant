#include "stdafx.h"
#include "common.h"
#include "spatterneditor.h"
using namespace SpatternEditor;

/* TRANSLATOR SpatternEditor::Editor */

enum ZValues
{
	zPianoRow,
	zCaret,
};

Editor::Editor(const Ptr<Spattern>& pattern)
: m_pattern(pattern)
{
	m_rulerView->setScene(&m_rulerScene);
	m_headView ->setScene(&m_headScene);
	m_bodyView ->setScene(&m_bodyScene);

	m_bodyScene.setSceneRect(0, 0, pattern->getLength(), 128);
	m_headScene.setSceneRect(0, 0, 1, 1000);

	setHZoom(64); setVZoom(16);

	for (int i=0; i<128; i++)
	{
		PianoKeyItem* kitem = new PianoKeyItem(this, i);
		m_headScene.addItem(kitem);
		m_pianoKeyItems.append(kitem);
		kitem->setPos(0, 127-i);

		PianoRowItem* ritem = new PianoRowItem(this, i);
		m_bodyScene.addItem(ritem);
		ritem->setPos(0, 127-i);
	}

	connect(m_headView, SIGNAL(signalResize(const QSize&, const QSize&)),
		this, SLOT(onHeadViewResize(const QSize&, const QSize&)) );

	for (int i=0; i<pattern->getLength(); i++)
	{
		QGraphicsRectItem* item = new QGraphicsRectItem(0,0,1,c_rulerHeight);
		m_rulerScene.addItem(item);
		m_rulerItems.append(item);
		item->setPos(i,0);
		item->setPen(Qt::NoPen);
		item->setBrush((i%2) ? Qt::white : QColor("azure"));
	}

	m_caretItem = new CaretItem(this, 128);
	m_bodyScene.addItem(m_caretItem);
}

void Editor::onHeadViewResize(const QSize& oldsize, const QSize& newsize)
{
	m_headView->resetTransform();
	m_headView->scale(m_headView->width(), m_vZoom);
}

void Editor::setHZoom(double hz)
{
	m_hZoom = hz;

	m_bodyView->resetTransform();
	m_bodyView->scale(m_hZoom, m_vZoom);

	m_rulerView->resetTransform();
	m_rulerView->scale(m_hZoom, 1);
}

void Editor::setVZoom(double vz)
{
	m_vZoom = vz;

	m_bodyView->resetTransform();
	m_bodyView->scale(m_hZoom, m_vZoom);

	m_headView->resetTransform();
	m_headView->scale(m_headView->width(), m_vZoom);
}

//////////////////////////////////////////////////////////////////////////

PianoKeyItem::PianoKeyItem(Editor* editor, int note)
: m_editor(editor), m_note(note), QGraphicsRectItem(0,0,1,1), m_isHighlighted(false)
{
	int notenum = note % 12; if (notenum < 0) notenum += 12;
	switch (notenum)
	{
	case 1: // C#
	case 3: // D#
	case 6: // F#
	case 8: // G#
	case 10: // A#
		m_isBlack = true;
		break;
	default:
		m_isBlack = false;
		break;
	}

	QLinearGradient gradient(0,0,1,0);
	gradient.setColorAt(0.3, m_isBlack ? Qt::black : Qt::gray);
	gradient.setColorAt(0.7, m_isHighlighted ? Qt::yellow : Qt::white);

	setBrush(gradient);
}

PianoRowItem::PianoRowItem(Editor* editor, int note)
: m_editor(editor), m_note(note), QGraphicsRectItem(0,0,editor->getPattern()->getLength(),1), m_isHighlighted(false)
{
	int notenum = note % 12; if (notenum < 0) notenum += 12;
	switch (notenum)
	{
	case 1: // C#
	case 3: // D#
	case 6: // F#
	case 8: // G#
	case 10: // A#
		m_isBlack = true;
		break;
	default:
		m_isBlack = false;
		break;
	}

	setBrush(m_isBlack ? QColor("azure") : Qt::white);
	setPen(Qt::NoPen);
	setZValue(zPianoRow);
}

///////////////////////////////////////////////////////////////////////////////////////

CaretItem::CaretItem(Editor* editor, qreal height)
: QGraphicsLineItem(0,0,0,height), m_blinkState(true)
{
	QPen pen(Qt::black, 3);
	pen.setCosmetic(true);
	setPen(pen);
	setZValue(zCaret);
	
	m_blinkTimer.setSingleShot(false);
	connect(&m_blinkTimer, SIGNAL(timeout()),
		this, SLOT(onBlinkTimer()) );
	m_blinkTimer.start(500);
}

void CaretItem::onBlinkTimer()
{
	m_blinkState = !m_blinkState;
	setOpacity(m_blinkState ? 1.0 : 0.2);
}
