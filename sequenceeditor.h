#pragma once

#include "sequence.h"
#include "mygraphicsview.h"
#include "headedview.h"
#include "mwtab.h"

namespace SequenceEditor
{
	class Editor;

	////////////////////////////////////////////////////////////////////////////

	class ClipItem;

	class TrackItem : public QObject, public QGraphicsRectItem
	{
		Q_OBJECT

	public:
		TrackItem(Editor* editor, const Ptr<Sequence::Track>& track);

		qreal height() { return rect().height(); }

	public slots:
		void onAddClip(const Ptr<Sequence::Clip>& clip);
		void onRemoveClip(const Ptr<Sequence::Clip>& clip);

	protected:
		Editor* m_editor;
		Ptr<Sequence::Track> m_track;

		QHash<Ptr<Sequence::Clip>, ClipItem*> m_clipItems;

		double getNearestSnapPoint(double x);

		virtual void mousePressEvent(QGraphicsSceneMouseEvent* ev);
		virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* ev);
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* ev);

		enum { none, drawNewClip } m_mouseMode;

		QGraphicsRectItem* m_newClipItem;
	};

	////////////////////////////////////////////////////////////////////////////

	class ClipItem : public QObject, public QGraphicsRectItem
	{
		Q_OBJECT

	public:
		ClipItem(Editor* editor, TrackItem* track, const Ptr<Sequence::Clip>& clip);

	protected:
		Editor* m_editor;
		TrackItem* m_track;
		Ptr<Sequence::Clip> m_clip;

		virtual void mousePressEvent(QGraphicsSceneMouseEvent* ev) { ev->accept(); }
		virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* ev);
	};

	////////////////////////////////////////////////////////////////////////////

	class RulerSectionItem : public QObject, public QGraphicsRectItem
	{
		Q_OBJECT

	public:
		RulerSectionItem(Editor* editor, const Ptr<Sequence::MasterTrackClip>& mtc);

		double getNearestSnapPoint(double x);

	protected:
		Editor* m_editor;
		Ptr<Sequence::MasterTrackClip> m_mtc;

		QColor m_colSmallGrid, m_colSmallGridLabel, m_colLargeGrid, m_colLargeGridLabel, m_colEven, m_colOdd;
		QFont m_bpmFont;

		void setupChildren();
		QList<QGraphicsRectItem*> m_beatRects;
		QMap<int, GraphicsSimpleTextItemWithBG*> m_gridLabels;
	};

	////////////////////////////////////////////////////////////////////////////

	class PlayLineItem : public QObject, public QGraphicsLineItem
	{
		Q_OBJECT

	public:
		PlayLineItem(Editor* editor, qreal height);

		void setHeight(qreal height);

	protected slots:
		void setPlayPos(double seconds);

	protected:
		Editor* m_editor;
	};

	class LoopMarkerItem : public QObject, public QGraphicsPathItem
	{
		Q_OBJECT

	public:
		enum Type {loopStart, loopEnd};

		LoopMarkerItem(Editor* editor, qreal height, Type type);

	protected:
		Editor* m_editor;
	};

	////////////////////////////////////////////////////////////////////////////

	class Editor : public HeadedView, public MWTab
	{
		Q_OBJECT

	public:
		Editor(const Ptr<Sequence::Seq>& seq, QWidget* parent);

		const Ptr<Sequence::Seq>& getSeq() { return m_seq; }

		qreal getBodyWidth();

		void setZoom(double pixelsPerSecond);

		QWidget* getMWTabWidget() { return this; }
		QString getTitle() { return tr("Sequence"); }
		QList<QAction*> getToolBarActions() { return m_toolBarActions; }

		enum Tool { none, move, createPattern };
		Tool getTool();

		double getNearestSnapPoint(double x);

	signals:
		void signalChangePlayPos(double seconds);

	protected slots:
		void onInsertTrack(int index, const Ptr<Sequence::Track>& track);
		void onRemoveTrack(int index, const Ptr<Sequence::Track>& track);

		void onPlayPosTimer();

	protected:
		Ptr<Sequence::Seq> m_seq;

		void setupToolBar();
		QList<QAction*> m_toolBarActions;
		QAction *m_actionToolMove, *m_actionToolCreatePattern;

		QGraphicsScene m_headScene, m_bodyScene, m_rulerScene;

		void createTrackItems();
		QList<TrackItem*> m_trackItems;

		void createRulerSectionItems();
		QMap<Ptr<Sequence::MasterTrackClip>, RulerSectionItem*> m_rulerSectionItems;

		QTimer m_playPosTimer;

		PlayLineItem *m_rulerPlayLine, *m_bodyPlayLine;
		LoopMarkerItem *m_loopStartItem, *m_loopEndItem;
	};
};
