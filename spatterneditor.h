#pragma once

#include "spattern.h"
#include "headedview.h"
#include "keyjazz.h"

namespace SpatternEditor
{
	class Editor;

	class PianoKeyItem : public QGraphicsRectItem
	{
	public:
		PianoKeyItem(Editor* editor, int note);

	protected:
		Editor* m_editor;
		int m_note;
		bool m_isBlack, m_isHighlighted;

		GraphicsLayoutTextItem* m_noteName;
	};

	class PianoRowItem : public QGraphicsRectItem
	{
	public:
		PianoRowItem(Editor* editor, int note);

	protected:
		Editor* m_editor;
		int m_note;
		bool m_isBlack, m_isHighlighted;
	};

	class CaretItem : public QObject, public QGraphicsLineItem
	{
		Q_OBJECT

	public:
		CaretItem(Editor* editor, qreal height);

		void setPos(double x);
		double getPos();

		void moveLeft();
		void moveRight();

	public slots:
		void onBlinkTimer();

	protected:
		Editor* m_editor;
		bool m_blinkState;
		QTimer m_blinkTimer;
	};

	class NoteItem : public QGraphicsRectItem
	{
	public:
		NoteItem(Editor* editor, const Ptr<Spattern::Note>& note);

	protected:
		Editor* m_editor;
		Ptr<Spattern::Note> m_note;
		GraphicsLayoutTextItem* m_text;
	};

	class Editor : public HeadedView
	{
		Q_OBJECT

	public:
		Editor(const Ptr<Spattern::Pattern>& pattern);

		Ptr<Spattern::Pattern> getPattern() const { return m_pattern; }

		double getHZoom() { return m_hZoom; }
		double getVZoom() { return m_vZoom; }
		void setHZoom(double hz);
		void setVZoom(double vz);

		void ensureCaretVisible(int margin = 50);

	protected slots:
		void onHeadViewResize(const QSize& oldsize, const QSize& newsize);

		void onNoteAdded(const Ptr<Spattern::Note>& note);
		void onNoteRemoved(const Ptr<Spattern::Note>& note);

	protected:
		Ptr<Spattern::Pattern> m_pattern;
		QGraphicsScene m_headScene, m_bodyScene, m_rulerScene;
		double m_hZoom, m_vZoom;

		QList<PianoKeyItem*> m_pianoKeyItems;
		QList<QGraphicsRectItem*> m_rulerItems;
		CaretItem* m_caret;

		void keyPressEvent(QKeyEvent* ev);

		void customEvent(QEvent* ev);
		void keyJazzEvent(KeyJazzEvent* ev);

		QHash<Ptr<Spattern::Note>, NoteItem*> m_noteItems;

	};
};
