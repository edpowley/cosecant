#include "stdafx.h"
#include "common.h"
#include "spatterneditor.h"
using namespace SpatternEditor;

#include "keyjazz.h"
#include "song.h"
#include "formatnote.h"

/* TRANSLATOR SpatternEditor::Editor */

enum ZValues
{
	zPianoRow,
	zNote,
	zCaret,
};

Editor::Editor(const Ptr<Spattern::Pattern>& pattern)
: m_pattern(pattern)
{
	m_rulerView->setScene(&m_rulerScene);
	m_headView ->setScene(&m_headScene);
	m_bodyView ->setScene(&m_bodyScene);

	m_bodyScene.setSceneRect(0, 0, pattern->getLength(), 128);
	m_headScene.setSceneRect(0, 0, 1, 128);
	m_rulerScene.setSceneRect(0, 0, pattern->getLength(), c_rulerHeight);

	m_bodyView->setHandleCursorKeys(false);

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

	for (int i=0; i<m_pattern->getLength(); i++)
	{
		QGraphicsRectItem* item = new QGraphicsRectItem(0,0,1,c_rulerHeight);
		m_rulerScene.addItem(item);
		m_rulerItems.append(item);
		item->setPos(i,0);
		item->setPen(Qt::NoPen);
		item->setBrush((i%2) ? Qt::white : QColor("azure"));
	}

	m_caret = new CaretItem(this, 128);
	m_bodyScene.addItem(m_caret);

	foreach(const Ptr<Spattern::Note>& note, m_pattern->getNotes())
	{
		NoteItem* item = new NoteItem(this, note);
		m_bodyScene.addItem(item);
		m_noteItems.insert(note, item);
	}

	connect(m_pattern, SIGNAL(signalNoteAdded(const Ptr<Spattern::Note>&)),
		this, SLOT(onNoteAdded(const Ptr<Spattern::Note>&)) );
	connect(m_pattern, SIGNAL(signalNoteRemoved(const Ptr<Spattern::Note>&)),
		this, SLOT(onNoteRemoved(const Ptr<Spattern::Note>&)) );
}

void Editor::onHeadViewResize(const QSize& oldsize, const QSize& newsize)
{
	m_headView->resetTransform();
	m_headView->scale(m_headView->width(), m_vZoom);
}

void Editor::onNoteAdded(const Ptr<Spattern::Note>& note)
{
	NoteItem* item = new NoteItem(this, note);
	m_bodyScene.addItem(item);
	m_noteItems.insert(note, item);
}

void Editor::onNoteRemoved(const Ptr<Spattern::Note>& note)
{
	if (m_noteItems.contains(note))
	{
		delete m_noteItems.take(note);
	}
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

void Editor::keyPressEvent(QKeyEvent* ev)
{
	switch (ev->key())
	{
	case Qt::Key_Left:
		m_caret->moveLeft();
		break;

	case Qt::Key_Right:
		m_caret->moveRight();
		break;

	default:
		KeyJazz::get().keyPress(ev);
	}
}

/////////////////////////////////////////////////////////////////////

class AddNoteCommand : public QUndoCommand
{
protected:
	Ptr<Spattern::Pattern> m_pattern;
	Ptr<Spattern::Note> m_note;

public:
	AddNoteCommand(const Ptr<Spattern::Pattern>& pattern, const Ptr<Spattern::Note>& note)
		: QUndoCommand(Editor::tr("add note")), m_pattern(pattern), m_note(note)
	{
	}

	void redo()
	{
		m_pattern->addNote(m_note);
	}

	void undo()
	{
		m_pattern->removeNote(m_note);
	}
};

void Editor::keyJazzEvent(KeyJazzEvent* ev)
{
	ev->accept();

	switch (ev->getType())
	{
	case KeyJazzEvent::noteOn:
		if (m_caret->getPos() < m_pattern->getLength())
		{
			Ptr<Spattern::Note> note = new Spattern::Note(m_pattern, m_caret->getPos(), 1.0, ev->getNote());
			theUndo().push(new AddNoteCommand(m_pattern, note));
			m_caret->moveRight();
			NoteItem* item = m_noteItems.value(note, NULL);
			if (item) m_bodyView->ensureVisible(item);
		}
		break;
	};
}

void Editor::customEvent(QEvent* ev)
{
	if (ev->type() == KeyJazzEvent::s_type)
	{
		keyJazzEvent(reinterpret_cast<KeyJazzEvent*>(ev));
	}
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

	m_noteName = new GraphicsLayoutTextItem(rect(), Qt::AlignRight | Qt::AlignVCenter,
		formatNote(m_note) + " ", this);
	m_noteName->setFont(QFont("Courier", 10));
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
: QGraphicsLineItem(0,0,0,height), m_editor(editor), m_blinkState(true)
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

void CaretItem::setPos(double x)
{
	x = clamp(x, 0.0, m_editor->getPattern()->getLength());
	QGraphicsLineItem::setPos(x,0);
	m_editor->ensureCaretVisible();
}

double CaretItem::getPos()
{
	return pos().x();
}

void CaretItem::moveLeft()
{
	setPos(ceil(getPos()) - 1);
}

void CaretItem::moveRight()
{
	setPos(floor(getPos()) + 1);
}

void Editor::ensureCaretVisible(int margin)
{
	int vscroll = m_bodyView->verticalScrollBar()->value();
	m_bodyView->ensureVisible(m_caret, margin);
	m_bodyView->verticalScrollBar()->setValue(vscroll);
}

/////////////////////////////////////////////////////////////////////

NoteItem::NoteItem(Editor* editor, const Ptr<Spattern::Note>& note)
: m_editor(editor), m_note(note)
{
	setRect(0, 0, m_note->getLength(), 1);
	setZValue(zNote);
	setPos(m_note->getStart(), 127 - m_note->getNote());
	setBrush(Qt::red);

	m_text = new GraphicsLayoutTextItem(rect(), Qt::AlignLeft | Qt::AlignVCenter,
		" " + formatNote(m_note->getNote()), this);
	m_text->setPen(QPen(Qt::yellow));
}
