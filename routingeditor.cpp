#include "stdafx.h"
#include "common.h"
#include "routingeditor.h"
using namespace RoutingEditor;

#include "song.h"
#include "theme.h"
#include "undo_command_ids.h"
#include "machinechooserwidget.h"

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

	setAcceptDrops(true);

	connect(
		m_routing,	SIGNAL(	signalAddMachine(const Ptr<Machine>&)),
		this,		SLOT(		onAddMachine(const Ptr<Machine>&))
	);

	connect(
		m_routing,	SIGNAL(	signalRemoveMachine(const Ptr<Machine>&)),
		this,		SLOT(		onRemoveMachine(const Ptr<Machine>&))
	);
}

void Editor::onAddMachine(const Ptr<Machine>& mac)
{
	MachineItem* mi = m_machineItemMap[mac] = new MachineItem(this, mac);
	mi->setZValue(10);
	m_scene.addItem(mi);
}

void Editor::onRemoveMachine(const Ptr<Machine>& mac)
{
	std::map<Ptr<Machine>, MachineItem*>::iterator iter = m_machineItemMap.find(mac);
	ASSERT(iter != m_machineItemMap.end());
	m_scene.removeItem(iter->second);
	delete iter->second;
}

QList<MachineItem*> Editor::getSelectedMachineItems()
{
	QList<MachineItem*> ret;
	foreach(QGraphicsItem* item, m_scene.selectedItems())
	{
		if (MachineItem* macitem = dynamic_cast<MachineItem*>(item))
			ret << macitem;
	}

	return ret;
}

////////////////////////////////////////////////////////////////////////////////////

bool Editor::shouldAcceptDropEvent(QDropEvent* ev)
{
	if (ev->mimeData()->hasFormat(MachineChooserWidget::c_dndMimeType)
		&& ev->source() // it comes from within this app
		&& ev->mimeData()->data(MachineChooserWidget::c_dndMimeType).size() > 0
	)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void Editor::dragEnterEvent(QDragEnterEvent* ev)
{
	QGraphicsView::dragEnterEvent(ev);

	if (shouldAcceptDropEvent(ev))
		ev->acceptProposedAction();
	else
		ev->ignore();
}

void Editor::dragMoveEvent(QDragMoveEvent* ev)
{
	QGraphicsView::dragMoveEvent(ev);

	if (shouldAcceptDropEvent(ev))
		ev->acceptProposedAction();
	else
		ev->ignore();
}

class AddMachineCommand : public QUndoCommand
{
public:
	AddMachineCommand(const Ptr<Routing>& routing, const Ptr<Machine>& mac)
		: m_routing(routing), m_mac(mac), QUndoCommand(Editor::tr("add machine '%1'").arg(mac->m_name))
	{
	}

	virtual void redo()
	{
		m_routing->addMachine(m_mac);
	}

	virtual void undo()
	{
		m_routing->removeMachine(m_mac);
	}

protected:
	Ptr<Routing> m_routing;
	Ptr<Machine> m_mac;
};

void Editor::dropEvent(QDropEvent* ev)
{
	QGraphicsView::dropEvent(ev);

	if (shouldAcceptDropEvent(ev))
	{
		QByteArray data = ev->mimeData()->data(MachineChooserWidget::c_dndMimeType);
		QStringList ids = QString::fromUtf8(data, data.size()).split("\n", QString::SkipEmptyParts);

		foreach(QString id, ids)
		{
			Ptr<Machine> mac = MachineFactory::get(id)->createMachine();
			mac->m_pos = mapToScene(ev->pos());
			theUndo().push(new AddMachineCommand(m_routing, mac));
		}

		ev->acceptProposedAction();
	}
	else
	{
		ev->ignore();
	}
}

//////////////////////////////////////////////////////////////////////////////

MachineItem::MachineItem(Editor* editor, const Ptr<Machine>& machine)
: m_mac(machine), m_editor(editor)
{
	setFlags(ItemIsSelectable);
	m_mouseMode = none;

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

	connect(m_mac, SIGNAL(signalChangePos()), this, SLOT(onMachinePosChanged()));
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

class MachineMoveCommand : public QUndoCommand
{
public:
	MachineMoveCommand(const Ptr<Machine>& mac, const QPointF& newpos)
		: m_mac(mac), m_newpos(newpos), m_oldpos(mac->m_pos)
	{ setText( QString("move machine '%1'").arg(m_mac->m_name) ); }

	virtual int id() const { return ucidMachineChangePos; }

	virtual void undo() { m_mac->setPos(m_oldpos); }
	virtual void redo() { m_mac->setPos(m_newpos); }

protected:
	Ptr<Machine> m_mac;
	QPointF m_oldpos, m_newpos;
};

void MachineItem::mousePressEvent(QGraphicsSceneMouseEvent* ev)
{
	QGraphicsRectItem::mousePressEvent(ev);

	if (m_mouseMode == none)
	{
		switch (ev->button())
		{
		case Qt::LeftButton:
			m_mouseMode = leftClick;
			break;
		}
	}
}

void MachineItem::mouseMoveEvent(QGraphicsSceneMouseEvent* ev)
{
	QGraphicsRectItem::mouseMoveEvent(ev);

	switch (m_mouseMode)
	{
	case leftClick:
		m_mouseMode = move;
		if (!isSelected()) setSelected(true);
		// no break -- fall through

	case move:
		{
			QPointF offset = ev->scenePos() - ev->lastScenePos();
			foreach(MachineItem* m, m_editor->getSelectedMachineItems())
			{
				m->setPos(m->pos() + offset);
			}
		}
		break;
	}
}

void MachineItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* ev)
{
	QGraphicsRectItem::mouseReleaseEvent(ev);

	switch (m_mouseMode)
	{
	case leftClick:
		m_mouseMode = none;
		break;

	case move:
		{
			QList<MachineItem*> selectedMachines = m_editor->getSelectedMachineItems();

			bool usemacro = (selectedMachines.length() > 1);
			if (usemacro) theUndo().beginMacro(tr("move machines"));

			foreach(MachineItem* m, selectedMachines)
			{
				theUndo().push(new MachineMoveCommand(m->m_mac, m->pos()));
			}
			
			if (usemacro) theUndo().endMacro();
		}
		m_mouseMode = none;
		break;
	}
}

void MachineItem::onMachinePosChanged()
{
	if (pos() != m_mac->m_pos)
		setPos(m_mac->m_pos);
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

	updatePath();

	QColor color = Theme::get().getSignalTypeColor(m_conn->getPin1()->m_type);
	QPen pen;
	pen.setColor(color);
	pen.setWidth(2);
	m_lineItem->setPen(pen);
	pen.setStyle(Qt::NoPen);
	m_triangleItem->setPen(pen);

	m_triangleItem->setBrush(QBrush(color));

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

	m_lineItem->setPath(path);

	qreal middle = path.percentAtLength(path.length() * 0.5);
	m_triangleItem->setPos(path.pointAtPercent(middle));
	m_triangleItem->setTransform(QTransform().rotate(-path.angleAtPercent(middle)));
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
