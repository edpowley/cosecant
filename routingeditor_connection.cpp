#include "stdafx.h"
#include "common.h"
#include "routingeditor.h"
using namespace RoutingEditor;

#include "theme.h"
#include "song.h"

/* TRANSLATOR RoutingEditor::Editor */

void Editor::onAddConnection(const Ptr<Connection>& conn)
{
	ConnectionItem* ci = m_connectionItemMap[conn] = new ConnectionItem(this, conn);
	m_scene.addItem(ci);
}

void Editor::onRemoveConnection(const Ptr<Connection>& conn)
{
	std::map<Ptr<Connection>, ConnectionItem*>::iterator iter = m_connectionItemMap.find(conn);
	ASSERT(iter != m_connectionItemMap.end());
	m_scene.removeItem(iter->second);
	delete iter->second;
}

/////////////////////////////////////////////////////////////////////////////////

class DisconnectCommand : public QUndoCommand
{
public:
	DisconnectCommand(const Ptr<Routing>& routing, const Ptr<Connection>& conn)
		: m_routing(routing), m_conn(conn), QUndoCommand(Editor::tr("disconnect"))
	{
	}

	virtual void redo()
	{
		m_routing->removeConnection(m_conn);
	}

	virtual void undo()
	{
		m_routing->addConnection(m_conn);
	}

protected:
	Ptr<Routing> m_routing;
	Ptr<Connection> m_conn;
};

void ConnectionItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* ev)
{
	QMenu menu;

	QAction* actDisconnect = menu.addAction(tr("&Disconnect"));
	
	QAction* action = menu.exec(ev->screenPos());
	if (action == actDisconnect)
	{
		theUndo().push(new DisconnectCommand(m_editor->getRouting(), m_conn));
	}
}

/////////////////////////////////////////////////////////////////////////////////

class AddConnectionCommand : public QUndoCommand
{
public:
	AddConnectionCommand(const Ptr<Routing>& routing, const Ptr<Connection>& conn)
		: m_routing(routing), m_conn(conn), QUndoCommand(Editor::tr("connect"))
	{
	}

	virtual void redo()
	{
		m_routing->addConnection(m_conn);
	}

	virtual void undo()
	{
		m_routing->removeConnection(m_conn);
	}

protected:
	Ptr<Routing> m_routing;
	Ptr<Connection> m_conn;
};

///////////////////////////////////////////////////////////////////////////////

ConnectionLineItem::ConnectionLineItem(Editor* editor)
: m_editor(editor)
{
	setZValue(20);
	m_lineItem = new QGraphicsPathItem(this);
	addToGroup(m_lineItem);

	m_triangleItem = new QGraphicsPathItem(this);
	addToGroup(m_triangleItem);

	QPainterPath tri;
	double a = 7.5;
	tri.moveTo( a, 0);
	tri.lineTo(-a, a);
	tri.lineTo(-a,-a);
	tri.closeSubpath();
	m_triangleItem->setPath(tri);
}

ConnectionItem::ConnectionItem(Editor* editor, const Ptr<Connection>& conn)
: m_conn(conn), ConnectionLineItem(editor)
{
	updatePath();
	updateColor();

	connect(m_editor->getPinItem(m_conn->getPin1()), SIGNAL(signalMove()), this, SLOT(updatePath()));
	connect(m_editor->getPinItem(m_conn->getPin2()), SIGNAL(signalMove()), this, SLOT(updatePath()));
}

void ConnectionLineItem::updatePath()
{
	prepareGeometryChange();

	Ptr<Pin> pin1 = getPin1();
	Ptr<Pin> pin2 = getPin2();

	QPointF pos1, pos2, off1, off2;

	if (pin1)
	{
		pos1 = m_editor->getPinItem(pin1)->scenePos();
		off1 = getPinBezierOffset(pin1->getSide());
	}
	else
	{
		pos1 = getPos1();
		off1 = QPointF(0,0);
	}

	if (pin2)
	{
		pos2 = m_editor->getPinItem(pin2)->scenePos();
		off2 = getPinBezierOffset(pin2->getSide());
	}
	else
	{
		pos2 = getPos2();
		off2 = QPointF(0,0);
	}

	QPainterPath path;
	path.moveTo(pos1);
	path.cubicTo(pos1 + off1, pos2 + off2, pos2);

	m_lineItem->setPath(path);

	qreal middle = path.percentAtLength(path.length() * 0.5);
	m_triangleItem->setPos(path.pointAtPercent(middle));
	m_triangleItem->setTransform(QTransform().rotate(-path.angleAtPercent(middle)));
}

void ConnectionLineItem::updateColor()
{
	CosecantAPI::SignalType::e signaltype;
	if		(getPin1()) signaltype = getPin1()->m_type;
	else if	(getPin2()) signaltype = getPin2()->m_type;
	else	return;

	QColor color = Theme::get().getSignalTypeColor(signaltype);
	QPen pen;
	pen.setColor(color);
	pen.setWidth(2);
	if (isFeedback())
		pen.setStyle(Qt::DashLine);
	else
		pen.setStyle(Qt::SolidLine);
	m_lineItem->setPen(pen);
	pen.setStyle(Qt::NoPen);
	m_triangleItem->setPen(pen);
	m_triangleItem->setBrush(QBrush(color));
}

QPointF ConnectionLineItem::getPinBezierOffset(Pin::Side side)
{
	double a = Editor::s_prefConnBezierOffset();

	switch (side)
	{
	case Pin::top:		return QPointF( 0,-a);
	case Pin::bottom:	return QPointF( 0, a);
	case Pin::left:		return QPointF(-a, 0);
	case Pin::right:	return QPointF( a, 0);
	default:	THROW_ERROR(Error, "Bad side");
	}
}

////////////////////////////////////////////////////////////////////////////

NewConnectionItem::NewConnectionItem(Editor* editor, const Ptr<Pin>& pin)
: ConnectionLineItem(editor), m_pinStart(pin)
{
	updatePath();
	updateColor();
}

Ptr<Pin> NewConnectionItem::getPin1()
{
	if (m_pinStart->m_direction == Pin::out)
		return m_pinStart;
	else
		return m_pinEnd;
}

Ptr<Pin> NewConnectionItem::getPin2()
{
	if (m_pinStart->m_direction == Pin::out)
		return m_pinEnd;
	else
		return m_pinStart;
}

void NewConnectionItem::onMouseMove(QGraphicsSceneMouseEvent* ev)
{
	m_mousePos = ev->scenePos();
	
	m_pinEnd = NULL;
	foreach(QGraphicsItem* item, m_editor->m_scene.items(m_mousePos))
	{
		if (PinItem* pinitem = dynamic_cast<PinItem*>(item))
		{
			Ptr<Pin> pin = pinitem->getPin();
			if (	pin->m_direction != m_pinStart->m_direction
				&&	pin->m_type == m_pinStart->m_type
			)
			{
				m_pinEnd = pin;
				break;
			}
		}
	}

	updatePath();
}

void NewConnectionItem::finish(QGraphicsSceneMouseEvent* ev)
{
	if (!m_pinEnd) return;

	try
	{
		Ptr<Connection> conn = m_editor->getRouting()->createConnection(m_pinStart, m_pinEnd);
		theUndo().push(new AddConnectionCommand(m_editor->getRouting(), conn));
	}
	catch (const Routing::CreateConnectionError& err)
	{
		QMessageBox msgbox;
		msgbox.setText(err.msg());
		msgbox.setIcon(QMessageBox::Critical);
		msgbox.exec();
	}
}
