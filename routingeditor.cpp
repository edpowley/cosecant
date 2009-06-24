#include "stdafx.h"
#include "common.h"
#include "routingeditor.h"
using namespace RoutingEditor;

#include "theme.h"

PrefsVar_Double Editor::s_prefPinSize("routingeditor/pinsize", 6);
PrefsVar_Double Editor::s_prefConnBezierOffset("routingeditor/connbezieroffset", 20);

Editor::Editor(const Ptr<Routing>& routing, QWidget* parent)
: m_routing(routing), QGraphicsView(parent)
{
	setScene(&m_scene);
	setRenderHints(QPainter::Antialiasing);
	setDragMode(RubberBandDrag);

	m_scene.setSceneRect(0,0,1000,1000);

//	m_scene.addRect(10,10,100,100);

	BOOST_FOREACH(const Ptr<Machine>& mac, m_routing->m_machines)
	{
		MachineItem* mi = m_machineItemMap[mac] = new MachineItem(this, mac);
		mi->setZValue(10);
		m_scene.addItem(mi);
	}

	BOOST_FOREACH(const Ptr<Machine>& mac, m_routing->m_machines)
	{
		BOOST_FOREACH(const Ptr<Pin>& pin, mac->m_outpins)
		{
			BOOST_FOREACH(Ptr<Connection>& conn, pin->m_connections)
			{
				ConnectionItem* ci = m_connectionItemMap[conn] = new ConnectionItem(this, conn);
				ci->setZValue(20);
				m_scene.addItem(ci);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////

MachineItem::MachineItem(Editor* editor, const Ptr<Machine>& machine)
: m_mac(machine), m_editor(editor)
{
	setFlags(ItemIsMovable | ItemIsSelectable);

	connect(this, SIGNAL(signalMove()), this, SLOT(onMove()));

	prepareGeometryChange();
	setPos(m_mac->m_pos);
	setRect(QRectF(-m_mac->m_halfsize, m_mac->m_halfsize));
	
	setBrush(QBrush(Theme::get().getMachineTypeHintColor(m_mac->m_colorhint)));
	
	QPen pen;
	pen.setColor(QColor("Black"));
	pen.setWidth(2);
	pen.setJoinStyle(Qt::MiterJoin);
	setPen(pen);

	m_selectedPen = pen;
	m_selectedPen.setColor(QColor("Yellow"));

	BOOST_FOREACH(const Ptr<Pin>& pin, m_mac->m_inpins)
	{
		PinItem* pi = editor->m_pinItemMap[pin] = new PinItem(editor, pin, this);
	}

	BOOST_FOREACH(const Ptr<Pin>& pin, m_mac->m_outpins)
	{
		PinItem* pi = editor->m_pinItemMap[pin] = new PinItem(editor, pin, this);
	}
}

void MachineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	//QGraphicsRectItem::paint(painter, option, widget);

	if (isSelected())
		painter->setPen(m_selectedPen);
	else
		painter->setPen(pen());

	painter->setBrush(brush());
	painter->drawRect(rect());

	QRectF r = rect();
	r.adjust(5,5,-5,-5);
	painter->setPen(pen());
	painter->drawText(r, Qt::AlignCenter | Qt::TextWordWrap, m_mac->m_name);
}

QVariant MachineItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
	switch (change)
	{
	case ItemPositionHasChanged:
		signalMove();
		break;
	}

	return QGraphicsRectItem::itemChange(change, value);
}

void MachineItem::onMove()
{
	// TODO: update machine pos
}

////////////////////////////////////////////////////////////////////////////

PinItem::PinItem(Editor* editor, const Ptr<Pin>& pin, MachineItem* parent)
: m_pin(pin), QGraphicsPathItem(parent), m_editor(editor)
{
	prepareGeometryChange();

	QPainterPath path;
	double d = Editor::s_prefPinSize() * 0.5;
	switch (m_pin->m_direction)
	{
	case Pin::in:
		path.moveTo(-d, -d);
		path.lineTo( 0,  d);
		path.lineTo( d, -d);
		break;

	case Pin::out:
		path.moveTo(-d,  d);
		path.lineTo( 0, -d);
		path.lineTo( d,  d);
		break;
	}

	setPath(path);

	QPointF pos = multElementWise(m_pin->m_machine->m_halfsize, m_pin->getPosOffset());
	setPos(pos);
	rotate(m_pin->getRotation());

	QPen pen;
	pen.setColor(Theme::get().getSignalTypeColor(m_pin->m_type));
	pen.setWidth(3);
	pen.setJoinStyle(Qt::MiterJoin);
	setPen(pen);

	connect(parent, SIGNAL(signalMove()), this, SIGNAL(signalMove()));
}

///////////////////////////////////////////////////////////////////////////////

ConnectionItem::ConnectionItem(Editor* editor, const Ptr<Connection>& conn)
: m_conn(conn), m_editor(editor)
{
	updatePath();

	QPen pen;
	pen.setColor(Theme::get().getSignalTypeColor(m_conn->getPin1()->m_type));
	pen.setWidth(2);
	setPen(pen);

	connect(m_editor->m_pinItemMap[m_conn->getPin1()], SIGNAL(signalMove()), this, SLOT(updatePath()));
	connect(m_editor->m_pinItemMap[m_conn->getPin2()], SIGNAL(signalMove()), this, SLOT(updatePath()));
}

void ConnectionItem::updatePath()
{
	prepareGeometryChange();

	QPointF pos1 = m_editor->m_pinItemMap[m_conn->getPin1()]->scenePos();
	QPointF pos2 = m_editor->m_pinItemMap[m_conn->getPin2()]->scenePos();
	QPointF off1 = getPinBezierOffset(m_conn->getPin1()->m_side);
	QPointF off2 = getPinBezierOffset(m_conn->getPin2()->m_side);

	QPainterPath path;
	path.moveTo(pos1);
	path.cubicTo(pos1 + off1, pos2 + off2, pos2);

	setPath(path);
}

QPointF ConnectionItem::getPinBezierOffset(Pin::Side side)
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
