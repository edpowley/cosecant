#pragma once

#include "spattern.h"
#include "headedview.h"

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

	public slots:
		void onBlinkTimer();

	protected:
		Editor* m_editor;
		bool m_blinkState;
		QTimer m_blinkTimer;
	};

	class Editor : public HeadedView
	{
		Q_OBJECT

	public:
		Editor(const Ptr<Spattern>& pattern);

		Ptr<Spattern> getPattern() const { return m_pattern; }

		double getHZoom() { return m_hZoom; }
		double getVZoom() { return m_vZoom; }
		void setHZoom(double hz);
		void setVZoom(double vz);

	protected slots:
		void onHeadViewResize(const QSize& oldsize, const QSize& newsize);

	protected:
		Ptr<Spattern> m_pattern;
		QGraphicsScene m_headScene, m_bodyScene, m_rulerScene;
		double m_hZoom, m_vZoom;

		QList<PianoKeyItem*> m_pianoKeyItems;
		QList<QGraphicsRectItem*> m_rulerItems;
		CaretItem* m_caretItem;
	};
};
