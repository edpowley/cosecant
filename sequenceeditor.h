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

		qreal height() { return rect().height(); }

	protected:
		Editor* m_editor;
		Ptr<Sequence::Track> m_track;
	};

	class RulerSectionItem : public QObject, public QGraphicsRectItem
	{
		Q_OBJECT

	public:
		RulerSectionItem(Editor* editor, const Ptr<Sequence::MasterTrackClip>& mtc);

	protected:
		Editor* m_editor;
		Ptr<Sequence::MasterTrackClip> m_mtc;

		QBrush m_brushBar, m_brushGrid, m_brushEven, m_brushOdd;

		void setupChildren();
		QList<QGraphicsRectItem*> m_beatRects;
		QList<GraphicsSimpleTextItemWithBG*> m_gridLabels;
	};

	class Editor : public QSplitter
	{
		Q_OBJECT

	public:
		Editor(const Ptr<Sequence::Seq>& seq, QWidget* parent);
		double m_pixelsPerSecond;

		qreal getBodyWidth();

		static const int c_rulerHeight = 50;

	protected slots:
		void onInsertTrack(int index, const Ptr<Sequence::Track>& track);
		void onRemoveTrack(int index, const Ptr<Sequence::Track>& track);

	protected:
		Ptr<Sequence::Seq> m_seq;

		MyGraphicsView *m_headView, *m_bodyView, *m_rulerView;
		QGraphicsScene m_headScene, m_bodyScene, m_rulerScene;

		void createTrackItems();
		QList<TrackItem*> m_trackItems;

		void createRulerSectionItems();
		QMap<Ptr<Sequence::MasterTrackClip>, RulerSectionItem*> m_rulerSectionItems;
	};
};
