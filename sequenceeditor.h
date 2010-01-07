#pragma once

#include "sequence.h"
#include "mygraphicsview.h"
#include "mwtab.h"

namespace SequenceEditor
{
	class Editor;

	/////////////////////////////////////////////////////////////////////////

	class TrackHeader : public QFrame
	{
	public:
		TrackHeader() { setFrameStyle(Panel | Raised); setLineWidth(2); }


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

	protected:
		Ptr<Sequence> m_seq;

		virtual void resizeEvent(QResizeEvent* ev);

		QScrollArea* m_trackHeaderScrollArea;
		QSplitter* m_trackHeaderParent;
		QHash< Ptr<Seq::Track>, TrackHeader* > m_trackHeaders;
	};
};
