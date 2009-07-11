#pragma once

#include "sequence.h"
#include "mygraphicsview.h"

namespace SequenceEditor
{
	class Editor;

	class TrackItem : public QObject, public QGraphicsRectItem
	{
		Q_OBJECT

	public:
		TrackItem(Editor* editor, const Ptr<Sequence::Track>& track);

	protected:
		Editor* m_editor;
		Ptr<Sequence::Track> m_track;
	};

	class Editor : public QSplitter
	{
		Q_OBJECT

	public:
		Editor(const Ptr<Sequence::Seq>& seq, QWidget* parent);

		qreal getBodyWidth();

	protected:
		Ptr<Sequence::Seq> m_seq;
		double m_pixelsPerSecond;

		MyGraphicsView *m_headView, *m_bodyView, *m_rulerView;
		QGraphicsScene m_headScene, m_bodyScene, m_rulerScene;

		void createTrackItems();
	};
};
