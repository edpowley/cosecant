#include "stdafx.h"
#include <QGraphicsSceneMouseEvent>
#include "common.h"
#include "routingeditor.h"
using namespace RoutingEditor;

#include "theme.h"

/* TRANSLATOR RoutingEditor::Editor */

PinItem* Editor::getPinItem(const Ptr<Pin>& pin)
{
	std::map<Ptr<Pin>, PinItem*>::iterator iter = m_pinItemMap.find(pin);
	if (iter != m_pinItemMap.end())
		return iter->second;
	else
		return NULL;
}

PinItem::PinItem(Editor* editor, const Ptr<Pin>& pin, MachineItem* parent)
: m_pin(pin), QGraphicsPathItem(parent), m_editor(editor), m_newConnectionItem(NULL)
{
	m_mouseMode = none;

	setToolTip(pin->m_name);

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
    QTransform transform;
    transform.rotate(m_pin->getRotation());
    setTransform(transform, true);
    //rotate(m_pin->getRotation());

	QPen pen;
	pen.setColor(Theme::get().getSignalTypeColor(m_pin->m_type));
	pen.setWidth(3);
	pen.setJoinStyle(Qt::MiterJoin);
	setPen(pen);

	connect(
		parent, SIGNAL(signalMove()),
		this, SIGNAL(signalMove()) );
	connect(
		m_pin, SIGNAL(signalPosChanged()),
		this, SLOT(onPinPosChanged()) );
}

void PinItem::onPinPosChanged()
{
	QPointF pos = multElementWise(m_pin->m_machine->m_halfsize, m_pin->getPosOffset());
	setPos(pos);

	QTransform trans; trans.rotate(m_pin->getRotation());
	setTransform(trans);

	signalMove();
}

QPainterPath PinItem::shape() const
{
	QPainterPath path;
	path.addRect(QGraphicsPathItem::shape().controlPointRect());
	return path;
}

void PinItem::mousePressEvent(QGraphicsSceneMouseEvent* ev)
{
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

void PinItem::mouseMoveEvent(QGraphicsSceneMouseEvent* ev)
{
	switch (m_mouseMode)
	{
	case leftClick:
		m_mouseMode = drawConnection;
		m_newConnectionItem = new NewConnectionItem(m_editor, m_pin);
		m_editor->m_scene.addItem(m_newConnectionItem);
		// no break -- fall through

	case drawConnection:
		m_newConnectionItem->onMouseMove(ev);
		break;
	}
}

void PinItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* ev)
{
	switch (m_mouseMode)
	{
	case leftClick:
		m_mouseMode = none;
		break;

	case drawConnection:
		m_newConnectionItem->finish(ev);
		m_editor->m_scene.removeItem(m_newConnectionItem);
		delete m_newConnectionItem;
		m_newConnectionItem = NULL;
		m_mouseMode = none;
		break;
	}
}
