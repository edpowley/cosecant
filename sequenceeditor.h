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

		QLabel* m_labelMachineName;
		QToolButton *m_btnMute, *m_btnSolo, *m_btnRecord;

	private:
		Ui::SequenceTrackHeader ui;

	};

	/////////////////////////////////////////////////////////////////////////

	class Track : public QObject, public QGraphicsRectItem
	{
		Q_OBJECT

	public:
		Track(Editor* editor, const Ptr<Seq::Track>& track);

	public slots:
		void setHeight(int height);
		void setY(int y);

	protected:
		Editor* m_editor;
		Ptr<Seq::Track> m_track;
	};

	/////////////////////////////////////////////////////////////////////////

	class Scene : public QGraphicsScene
	{
		Q_OBJECT

	public:
		Scene(Editor* editor) : m_editor(editor) {}

	protected:
		Editor* m_editor;
		virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* ev);
		virtual void dragMoveEvent(QGraphicsSceneDragDropEvent* ev);
		virtual void dropEvent(QGraphicsSceneDragDropEvent* ev);

		bool shouldAcceptDropEvent(QGraphicsSceneDragDropEvent* ev);
	};

	/////////////////////////////////////////////////////////////////////////

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

	protected slots:
		void onTrackAdded(int index, const Ptr<Seq::Track>& track);
		void onTrackRemoved(int index, const Ptr<Seq::Track>& track);

	protected:
		Ptr<Sequence> m_seq;

		virtual void resizeEvent(QResizeEvent* ev);

		QScrollArea* m_trackHeaderScrollArea;
		QWidget* m_trackHeaderParent;
		QVBoxLayout* m_trackHeaderLayout;
		QHash< Ptr<Seq::Track>, TrackHeader* > m_trackHeaders;
		QHash< Ptr<Seq::Track>, Track* > m_tracks;
	};
};
