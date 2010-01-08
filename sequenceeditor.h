#pragma once

#include "sequence.h"
#include "mygraphicsview.h"
#include "mwtab.h"
#include "ui_sequenceeditor_trackheader.h"

namespace SequenceEditor
{
	class Editor;

	/////////////////////////////////////////////////////////////////////////

	class TrackHeader : public QWidget
	{
		Q_OBJECT

	public:
		TrackHeader(Editor* editor, const Ptr<Seq::Track>& track);

	signals:
		void signalHeightChanged(int height);
		void signalYChanged(int y);

	protected slots:

	protected:
		Editor* m_editor;
		Ptr<Seq::Track> m_track;

		virtual void resizeEvent(QResizeEvent* ev);
		virtual void moveEvent(QMoveEvent* ev);

		void forceHeight(int h) { setMinimumHeight(h); setMaximumHeight(h); }

	private:
		Ui::SequenceTrackHeader ui;

	};

	/////////////////////////////////////////////////////////////////////////

	class Track : public QObject, public QGraphicsRectItem
	{
		Q_OBJECT

	public:
		Track(Editor* editor, const Ptr<Seq::Track>& track);

		const Ptr<Seq::Track>& getTrack() { return m_track; }

	public slots:
		void setHeight(int height);
		void setY(int y);

	protected:
		Editor* m_editor;
		Ptr<Seq::Track> m_track;

		virtual void mousePressEvent(QGraphicsSceneMouseEvent* ev);
		virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* ev);
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* ev);
	};

	/////////////////////////////////////////////////////////////////////////

	class Scene : public QGraphicsScene
	{
		Q_OBJECT

	public:
		Scene(Editor* editor);

	protected slots:
		void setHeight(int h);

	protected:
		Editor* m_editor;
		virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* ev);
		virtual void dragMoveEvent(QGraphicsSceneDragDropEvent* ev);
		virtual void dropEvent(QGraphicsSceneDragDropEvent* ev);

		bool shouldAcceptDropEvent(QGraphicsSceneDragDropEvent* ev);
	};

	/////////////////////////////////////////////////////////////////////////

	class TrackHeaderParent : public QWidget
	{
		Q_OBJECT

	signals:
		void signalHeightChanged(int h);

	protected:
		virtual void resizeEvent(QResizeEvent* ev) { signalHeightChanged(ev->size().height()); }
	};

	class Editor : public MyGraphicsView, public MWTab
	{
		Q_OBJECT

	public:
		QWidget* getMWTabWidget() { return this; }
		QString getTitle() { return tr("Sequence"); }
		QWidget* getPalette() { return NULL; }

		Editor(const Ptr<Sequence>& seq, QWidget* parent);
		Scene m_scene;
		Ptr<Sequence> getSequence() { return m_seq; }

		static PrefsVar_Int s_prefTrackHeaderWidth, s_prefRulerHeight;

		double getSceneWidth() { return 240; }

		void getSelectionRange(int64& start, int64& end) { start = m_selStart; end = m_selEnd; }

		void startSelectionDrag(const Ptr<Seq::Track>& track, QGraphicsSceneMouseEvent* ev);
		void continueSelectionDrag(QGraphicsSceneMouseEvent* ev);
		void endSelectionDrag(QGraphicsSceneMouseEvent* ev);

	protected slots:
		void onTrackAdded(int index, const Ptr<Seq::Track>& track);
		void onTrackRemoved(int index, const Ptr<Seq::Track>& track);

	protected:
		Ptr<Sequence> m_seq;

		virtual void resizeEvent(QResizeEvent* ev);

		QScrollArea* m_trackHeaderScrollArea;
		TrackHeaderParent* m_trackHeaderParent;
		QVBoxLayout* m_trackHeaderLayout;
		QHash< Ptr<Seq::Track>, TrackHeader* > m_trackHeaders;
		QHash< Ptr<Seq::Track>, Track* > m_tracks;

		int64 m_selStart, m_selEnd;
		int m_selFirstTrack, m_selLastTrack;

		QGraphicsRectItem* m_itemSelection;
		void updateSelectionItem();

		int64 m_selDragStart;
		int m_selDragStartTrack;

		Track* getTrackAtY(double y);
	};
};
