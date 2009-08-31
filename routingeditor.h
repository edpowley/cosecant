#pragma once

#include "routing.h"
#include "mygraphicsview.h"
#include "mwtab.h"
#include "machinechooserwidget.h"

namespace RoutingEditor
{
	class Editor;

	class MachineItem : public QObject, public QGraphicsRectItem
	{
		Q_OBJECT

	public:
		MachineItem(Editor* editor, const Ptr<Machine>& machine);

		virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

	protected:
		Ptr<Machine> m_mac;
		Editor* m_editor;
		QPen m_selectedPen;

		virtual void mousePressEvent(QGraphicsSceneMouseEvent* ev);
		virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* ev);
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* ev);
		virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* ev);

		virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* ev);

		void deleteMachine();

		enum { none, leftClick, rightClick, move } m_mouseMode;

		virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);

	signals:
		void signalMove();

	protected slots:
		void onMachinePosChanged();
		void slotUpdate() { update(); }
		void onMachineAddPin(const Ptr<Pin>& pin);
		void onMachineRemovePin(const Ptr<Pin>& pin);
	};

	/////////////////////////////////////////////////////////////////////////

	class NewConnectionItem;

	class PinItem : public QObject, public QGraphicsPathItem
	{
		Q_OBJECT

	public:
		PinItem(Editor* editor, const Ptr<Pin>& pin, MachineItem* parent);

		Ptr<Pin> getPin() { return m_pin; }

		virtual QPainterPath shape() const;

	protected slots:
		void onPinPosChanged();

	protected:
		Ptr<Pin> m_pin;
		Editor* m_editor;

		virtual void mousePressEvent(QGraphicsSceneMouseEvent* ev);
		virtual void mouseMoveEvent(QGraphicsSceneMouseEvent* ev);
		virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* ev);

		enum { none, leftClick, drawConnection } m_mouseMode;
		NewConnectionItem* m_newConnectionItem;

	signals:
		void signalMove();
	};

	/////////////////////////////////////////////////////////////////////////

	class ConnectionLineItem : public QObject, public QGraphicsItemGroup
	{
		Q_OBJECT

	public:
		ConnectionLineItem(Editor* editor);

	protected:
		Editor* m_editor;
		virtual Ptr<Pin> getPin1() = 0;
		virtual Ptr<Pin> getPin2() = 0;
		virtual QPointF getPos1() { return QPointF(0,0); }
		virtual QPointF getPos2() { return QPointF(0,0); }
		virtual bool isFeedback() = 0;

		QGraphicsPathItem* m_lineItem;
		QGraphicsPathItem* m_triangleItem;

		QPointF getPinBezierOffset(Pin::Side side);

	protected slots:
		void updatePath();
		void updateColor();
	};

	class ConnectionItem : public ConnectionLineItem
	{
		Q_OBJECT

	public:
		ConnectionItem(Editor* editor, const Ptr<Connection>& conn);

		virtual bool isFeedback() { return m_conn->m_feedback; }

	protected:
		Ptr<Connection> m_conn;
		virtual Ptr<Pin> getPin1() { return m_conn->getPin1(); }
		virtual Ptr<Pin> getPin2() { return m_conn->getPin2(); }

		virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent* ev);
	};

	class NewConnectionItem : public ConnectionLineItem
	{
		Q_OBJECT

	public:
		NewConnectionItem(Editor* editor, const Ptr<Pin>& pin);

		virtual bool isFeedback() { return false; }

		void onMouseMove(QGraphicsSceneMouseEvent* ev);
		void finish(QGraphicsSceneMouseEvent* ev);

	protected:
		Ptr<Pin> m_pinStart, m_pinEnd;
		QPointF m_mousePos;

		virtual Ptr<Pin> getPin1();
		virtual Ptr<Pin> getPin2();
		virtual QPointF getPos1() { return m_mousePos; }
		virtual QPointF getPos2() { return m_mousePos; }
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

		friend MachineItem;
		friend PinItem;
		friend Scene;

	public:
		QWidget* getMWTabWidget() { return this; }
		QString getTitle() { return tr("Routing"); }
		QWidget* getPalette() { return s_palette; }

		Editor(const Ptr<Routing>& routing, QWidget* parent);
		Scene m_scene;
		Ptr<Routing> getRouting() { return m_routing; }

		static PrefsVar_Double s_prefPinSize, s_prefConnBezierOffset;

		QList<MachineItem*> getSelectedMachineItems();

		PinItem* getPinItem(const Ptr<Pin>& pin);

	protected:
		Ptr<Routing> m_routing;

		std::map< Ptr<Machine>,		MachineItem*	> m_machineItemMap;
		std::map< Ptr<Pin>,			PinItem*		> m_pinItemMap;
		std::map< Ptr<Connection>,	ConnectionItem*	> m_connectionItemMap;

		static MachineChooserWidget* s_palette; // shared between all routing editors

	protected slots:
		void onAddMachine(const Ptr<Machine>& mac);
		void onRemoveMachine(const Ptr<Machine>& mac);
		void onAddConnection(const Ptr<Connection>& conn);
		void onRemoveConnection(const Ptr<Connection>& conn);
	};
};
