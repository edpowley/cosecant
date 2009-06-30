#pragma once

#include "routing.h"

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

		enum { none, leftClick, move } m_mouseMode;

		virtual QVariant itemChange(GraphicsItemChange change, const QVariant& value);

	signals:
		void signalMove();

	protected slots:
		void onMachinePosChanged();
	};

	class PinItem : public QObject, public QGraphicsPathItem
	{
		Q_OBJECT

	public:
		PinItem(Editor* editor, const Ptr<Pin>& pin, MachineItem* parent);

	protected:
		Ptr<Pin> m_pin;
		Editor* m_editor;

	signals:
		void signalMove();
	};

	class ConnectionItem : public QObject, public QGraphicsItemGroup
	{
		Q_OBJECT

	public:
		ConnectionItem(Editor* editor, const Ptr<Connection>& conn);

	protected:
		Ptr<Connection> m_conn;
		Editor* m_editor;

		QGraphicsPathItem* m_lineItem;
		QGraphicsPathItem* m_triangleItem;

		QPointF getPinBezierOffset(Pin::Side side);

		virtual void mousePressEvent(QGraphicsSceneMouseEvent* ev)
		{
			qDebug("connection mouse press"); QGraphicsItemGroup::mousePressEvent(ev);
		}

	protected slots:
		void updatePath();
	};

	/////////////////////////////////////////////////////////////////////////

	class Editor : public QGraphicsView
	{
		Q_OBJECT

		friend MachineItem;
		friend PinItem;
		friend ConnectionItem;

	public:
		Editor(const Ptr<Routing>& routing, QWidget* parent);

		static PrefsVar_Double s_prefPinSize, s_prefConnBezierOffset;

		QList<MachineItem*> getSelectedMachineItems();

	protected:
		Ptr<Routing> m_routing;
		QGraphicsScene m_scene;

		std::map< Ptr<Machine>,		MachineItem*	> m_machineItemMap;
		std::map< Ptr<Pin>,			PinItem*		> m_pinItemMap;
		std::map< Ptr<Connection>,	ConnectionItem*	> m_connectionItemMap;

		virtual void dragEnterEvent(QDragEnterEvent* ev);
		virtual void dragMoveEvent(QDragMoveEvent* ev);
		virtual void dropEvent(QDropEvent* ev);

		bool shouldAcceptDropEvent(QDropEvent* ev);

	protected slots:
		void onAddMachine(const Ptr<Machine>& mac);
		void onRemoveMachine(const Ptr<Machine>& mac);
	};
};
