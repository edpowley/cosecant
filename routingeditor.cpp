#include "stdafx.h"
#include "common.h"
#include "routingeditor.h"
using namespace RoutingEditor;

/* TRANSLATOR RoutingEditor::Editor */

#include "song.h"
#include "theme.h"
#include "undo_command_ids.h"
#include "machinechooserwidget.h"
#include "cosecantmainwindow.h"
#include "parameditor.h"
#include "dlg_machinerename.h"

PrefsVar_Double Editor::s_prefPinSize("routingeditor/pinsize", 6);
PrefsVar_Double Editor::s_prefConnBezierOffset("routingeditor/connbezieroffset", 50);

Editor::Editor(const Ptr<Routing>& routing, QWidget* parent)
: m_routing(routing), MyGraphicsView(parent), m_scene(this)
{
	setScene(&m_scene);
	setDragMode(RubberBandDrag);

	m_scene.setSceneRect(0,0,1000,1000);

	BOOST_FOREACH(const Ptr<Machine>& mac, m_routing->m_machines)
	{
		MachineItem* mi = m_machineItemMap[mac] = new MachineItem(this, mac);
		m_scene.addItem(mi);
	}

	BOOST_FOREACH(const Ptr<Machine>& mac, m_routing->m_machines)
	{
		BOOST_FOREACH(const Ptr<Pin>& pin, mac->m_outpins)
		{
			BOOST_FOREACH(Ptr<Connection>& conn, pin->m_connections)
			{
				ConnectionItem* ci = m_connectionItemMap[conn] = new ConnectionItem(this, conn);
				m_scene.addItem(ci);
			}
		}
	}

	connect(
		m_routing,	SIGNAL(	signalAddMachine(const Ptr<Machine>&)),
		this,		SLOT(		onAddMachine(const Ptr<Machine>&))
	);

	connect(
		m_routing,	SIGNAL(	signalRemoveMachine(const Ptr<Machine>&)),
		this,		SLOT(		onRemoveMachine(const Ptr<Machine>&))
	);

	connect(
		m_routing,	SIGNAL(	signalAddConnection(const Ptr<Connection>&)),
		this,		SLOT(		onAddConnection(const Ptr<Connection>&))
	);

	connect(
		m_routing,	SIGNAL(	signalRemoveConnection(const Ptr<Connection>&)),
		this,		SLOT(		onRemoveConnection(const Ptr<Connection>&))
	);
}

PinItem* Editor::getPinItem(const Ptr<Pin>& pin)
{
	std::map<Ptr<Pin>, PinItem*>::iterator iter = m_pinItemMap.find(pin);
	if (iter != m_pinItemMap.end())
		return iter->second;
	else
		return NULL;
}

void Editor::onAddMachine(const Ptr<Machine>& mac)
{
	MachineItem* mi = m_machineItemMap[mac] = new MachineItem(this, mac);
	m_scene.addItem(mi);
}

void Editor::onRemoveMachine(const Ptr<Machine>& mac)
{
	std::map<Ptr<Machine>, MachineItem*>::iterator iter = m_machineItemMap.find(mac);
	ASSERT(iter != m_machineItemMap.end());
	m_scene.removeItem(iter->second);
	delete iter->second;
}

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

bool Scene::shouldAcceptDropEvent(QGraphicsSceneDragDropEvent* ev)
{
	QGraphicsItem* item = itemAt(ev->scenePos());
	if (item) return false;

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

void Scene::dragEnterEvent(QGraphicsSceneDragDropEvent* ev)
{
	if (shouldAcceptDropEvent(ev))
		ev->acceptProposedAction();
	else
		ev->ignore();
}

void Scene::dragMoveEvent(QGraphicsSceneDragDropEvent* ev)
{
	if (shouldAcceptDropEvent(ev))
		ev->acceptProposedAction();
	else
		ev->ignore();
}

class AddMachineCommand : public QUndoCommand
{
public:
	AddMachineCommand(const Ptr<Routing>& routing, const Ptr<Machine>& mac)
		: m_routing(routing), m_mac(mac), QUndoCommand(Editor::tr("add machine '%1'").arg(mac->getName()))
	{
		if (m_mac->getInfo()->flags & CosecantAPI::MachineFlags::createSequenceTrack)
		{
			m_seq = Song::get().m_sequence;
			m_seqTrack = new Sequence::Track(m_mac);
			m_seqTrackIndex = m_seq->m_tracks.length();
		}
	}

	virtual void redo()
	{
		m_routing->addMachine(m_mac);
		if (m_seqTrack) m_seq->insertTrack(m_seqTrackIndex, m_seqTrack);
	}

	virtual void undo()
	{
		if (m_seqTrack) m_seq->removeTrack(m_seqTrackIndex);
		m_routing->removeMachine(m_mac);
	}

protected:
	Ptr<Routing> m_routing;
	Ptr<Machine> m_mac;
	Ptr<Sequence::Seq> m_seq;
	Ptr<Sequence::Track> m_seqTrack;
	int m_seqTrackIndex;
};

void Scene::dropEvent(QGraphicsSceneDragDropEvent* ev)
{
	if (shouldAcceptDropEvent(ev))
	{
		QByteArray data = ev->mimeData()->data(MachineChooserWidget::c_dndMimeType);
		QStringList ids = QString::fromUtf8(data, data.size()).split("\n", QString::SkipEmptyParts);

		foreach(QString id, ids)
		{
			Ptr<Machine> mac = MachineFactory::get(id)->createMachine();
			mac->m_pos = ev->scenePos();
			theUndo().push(new AddMachineCommand(m_editor->m_routing, mac));
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
	setZValue(10);
	setFlags(ItemIsSelectable);
	m_mouseMode = none;

	prepareGeometryChange();
	setPos(m_mac->m_pos);
	setRect(QRectF(-m_mac->m_halfsize, m_mac->m_halfsize));
	
	QPen pen;
	pen.setColor(Qt::black);
	pen.setWidth(2);
	pen.setJoinStyle(Qt::MiterJoin);
	setPen(pen);

	m_selectedPen = pen;
	m_selectedPen.setColor(Qt::yellow);

	BOOST_FOREACH(const Ptr<Pin>& pin, m_mac->m_inpins)
	{
		PinItem* pi = editor->m_pinItemMap[pin] = new PinItem(editor, pin, this);
	}

	BOOST_FOREACH(const Ptr<Pin>& pin, m_mac->m_outpins)
	{
		PinItem* pi = editor->m_pinItemMap[pin] = new PinItem(editor, pin, this);
	}

	connect(
		m_mac, SIGNAL(signalChangePos()),
		this, SLOT(onMachinePosChanged()) );
	connect(
		m_mac, SIGNAL(signalRename(const QString&)),
		this, SLOT(slotUpdate()) );
	connect(
		m_mac, SIGNAL(signalChangeAppearance()),
		this, SLOT(slotUpdate()) );
	connect(
		m_mac, SIGNAL(signalAddPin(const Ptr<Pin>&)),
		this, SLOT(onMachineAddPin(const Ptr<Pin>&)) );
	connect(
		m_mac, SIGNAL(signalRemovePin(const Ptr<Pin>&)),
		this, SLOT(onMachineRemovePin(const Ptr<Pin>&)) );
}

void MachineItem::onMachineAddPin(const Ptr<Pin> &pin)
{
	m_editor->m_pinItemMap[pin] = new PinItem(m_editor, pin, this);
}

void MachineItem::onMachineRemovePin(const Ptr<Pin>& pin)
{
	std::map<Ptr<Pin>, PinItem*>::iterator iter = m_editor->m_pinItemMap.find(pin);
	if (iter != m_editor->m_pinItemMap.end())
	{
		delete iter->second;
		m_editor->m_pinItemMap.erase(iter);
	}
}

void MachineItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	//QGraphicsRectItem::paint(painter, option, widget);

	if (isSelected())
		painter->setPen(m_selectedPen);
	else
		painter->setPen(pen());

	// Leaving the following line as a warning. Don't do setBrush inside paint. It triggers a repaint,
	// and recursion ensues. Idiot.
	//setBrush(m_mac->getColor());

	painter->setBrush(m_mac->getColor());
	painter->drawRect(rect());

	QRectF r = rect();
	r.adjust(5,5,-5,-5);
	painter->setPen(pen());
	painter->drawText(r, Qt::AlignCenter | Qt::TextWordWrap, m_mac->getName());
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
	{ setText( QString("move machine '%1'").arg(m_mac->getName()) ); }

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

		case Qt::RightButton:
			m_mouseMode = rightClick;
			ev->accept();
			if (!isSelected())
			{
				if (!(ev->modifiers() & Qt::ControlModifier))
					scene()->clearSelection();
				setSelected(true);
			}

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
		// no break -- fall through to case move

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
		if (ev->button() == Qt::LeftButton)
		{
			m_mouseMode = none;
		}
		break;

	case rightClick:
		if (ev->button() == Qt::RightButton)
		{
			ev->accept();
			m_mouseMode = none;
		}
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

void MachineItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* ev)
{
	m_mac->showParamEditor();
}

/////////////////////////////////////////////////////////////////////////////

class DeleteMachineCommand : public QUndoCommand
{
public:
	DeleteMachineCommand(const Ptr<Routing>& routing, const Ptr<Machine>& mac)
		: m_routing(routing), m_mac(mac), QUndoCommand(Editor::tr("delete machine '%1'").arg(mac->getName()))
	{
		m_seq = Song::get().m_sequence;
		m_seqTracks = m_seq->getTracksForMachine(m_mac);

		BOOST_FOREACH(Ptr<Pin>& pin, m_mac->m_inpins)
			BOOST_FOREACH(Ptr<Connection>& conn, pin->m_connections)
				m_conns.insert(conn);

		BOOST_FOREACH(Ptr<Pin>& pin, m_mac->m_outpins)
			BOOST_FOREACH(Ptr<Connection>& conn, pin->m_connections)
				m_conns.insert(conn);
	}

	virtual void redo()
	{
		m_seq->removeTracks(m_seqTracks);

		Routing::ChangeBatch batch(m_routing);

		BOOST_FOREACH(Ptr<Connection>& conn, m_conns)
			m_routing->removeConnection(conn);

		m_routing->removeMachine(m_mac);
	}

	virtual void undo()
	{
		Routing::ChangeBatch batch(m_routing);

		m_routing->addMachine(m_mac);

		BOOST_FOREACH(Ptr<Connection>& conn, m_conns)
			m_routing->addConnection(conn);

		m_seq->insertTracks(m_seqTracks);
	}

protected:
	Ptr<Routing> m_routing;
	Ptr<Machine> m_mac;
	std::set< Ptr<Connection> > m_conns;
	Ptr<Sequence::Seq> m_seq;
	Sequence::Seq::TrackIndexMap m_seqTracks;
};

class RenameMachineCommand : public QUndoCommand
{
public:
	RenameMachineCommand(const Ptr<Machine>& mac, const QString& newname, MachineTypeHint::e newcolorhint)
		: m_mac(mac), m_oldname(mac->getName()), m_newname(newname),
		m_oldcolorhint(mac->getColorHint()), m_newcolorhint(newcolorhint)
	{
		if (m_oldname != m_newname)
		{
			QString basetext;
			if (m_oldcolorhint != m_newcolorhint)
				basetext = Editor::tr("rename machine '%1' to '%2' and change appearance");
			else
				basetext = Editor::tr("rename machine '%1' to '%2'");

			setText(basetext.arg(m_oldname).arg(m_newname));
		}
		else
		{
			setText(Editor::tr("change appearance of machine '%1'").arg(m_oldname));
		}
	}

	virtual void redo()
	{
		m_mac->setName(m_newname);
		m_mac->setColorHint(m_newcolorhint);
	}

	virtual void undo()
	{
		m_mac->setName(m_oldname);
		m_mac->setColorHint(m_oldcolorhint);
	}

protected:
	Ptr<Machine> m_mac;
	QString m_oldname, m_newname;
	MachineTypeHint::e m_oldcolorhint, m_newcolorhint;
};

class AddParamPinCommand : public QUndoCommand
{
public:
	AddParamPinCommand(const Ptr<Machine>& mac, const Ptr<Parameter::Scalar>& param, TimeUnit::e timeUnit)
		: m_mac(mac), m_param(param), QUndoCommand(Editor::tr("add pin for parameter '%1'").arg(param->getName()))
	{
		m_pin = new ParamPin(m_param, timeUnit);
		m_pin->setPos(2.0f);
		m_pin->setSide(Pin::top);
		m_pin->m_name = Editor::tr("Parameter '%1'").arg(m_param->getName());
	}

	virtual void redo()
	{
		m_mac->addPin(m_pin);
		m_param->setParamPin(m_pin);
	}

	virtual void undo()
	{
		m_param->unsetParamPin();
		m_mac->removePin(m_pin);
	}

protected:
	Ptr<Machine> m_mac;
	Ptr<Parameter::Scalar> m_param;
	Ptr<ParamPin> m_pin;
};

class RemoveParamPinCommand : public QUndoCommand
{
public:
	RemoveParamPinCommand(const Ptr<Machine>& mac, const Ptr<Parameter::Base>& param)
		: m_routing(mac->m_routing), m_mac(mac), m_param(param), m_pin(param->getParamPin()),
		QUndoCommand(Editor::tr("remove pin for parameter '%1'").arg(param->getName()))
	{
		m_conns = m_pin->m_connections;
	}

	virtual void redo()
	{
		Routing::ChangeBatch batch(m_routing);

		foreach(const Ptr<Connection>& conn, m_conns)
		{
			m_routing->removeConnection(conn);
		}

		m_param->unsetParamPin();
		m_mac->removePin(m_pin);
	}

	virtual void undo()
	{
		Routing::ChangeBatch batch(m_routing);

		m_mac->addPin(m_pin);
		m_param->setParamPin(m_pin);

		foreach(const Ptr<Connection>& conn, m_conns)
		{
			m_routing->addConnection(conn);
		}
	}

protected:
	Ptr<Routing> m_routing;
	Ptr<Machine> m_mac;
	Ptr<Parameter::Base> m_param;
	Ptr<ParamPin> m_pin;
	QList< Ptr<Connection> > m_conns;
};


QMenu* Parameter::Group::populateParamPinMenu(QMenu* menu, QMap<QAction*, Parameter::ParamPinSpec>& actions)
{
	QMenu* submenu = menu->addMenu(m_name);
	foreach(Base* p, m_params)
	{
		p->populateParamPinMenu(submenu, actions);
	}
	return submenu;
}

QMenu* Parameter::Scalar::populateParamPinMenu(QMenu* menu, QMap<QAction*, Parameter::ParamPinSpec>& actions)
{
	QAction* act = menu->addAction(m_name);
	act->setCheckable(true);
	act->setChecked(getParamPin() != NULL);
	actions.insert(act, ParamPinSpec(this));
	return NULL;
}

QMenu* Parameter::Time::populateParamPinMenu(QMenu* menu, QMap<QAction*, Parameter::ParamPinSpec>& actions)
{
	if (getParamPin() != NULL)
	{
		TimeUnit::e unit = getParamPin()->getTimeUnit();
		QString unitName = ParamEditorWidget::TimeUnitCombo::getUnitName(unit);
		QAction* act = menu->addAction( QString("%1 (%2)").arg(m_name).arg(unitName) );
		act->setCheckable(true);
		act->setChecked(true);
		actions.insert(act, ParamPinSpec(this, unit));
	}
	else
	{
		QMenu* submenu = menu->addMenu(m_name);
		submenu->menuAction()->setCheckable(true);
		submenu->menuAction()->setChecked(false);
		for (int i=0; i<TimeUnit::numUnits; i++)
		{
			if (m_displayUnits & (1 << i))
			{
				TimeUnit::e unit = static_cast<TimeUnit::e>(1 << i);
				QAction* act = submenu->addAction(ParamEditorWidget::TimeUnitCombo::getUnitName(unit));

				if (unit == m_displayUnit)
					submenu->setDefaultAction(act);

				actions.insert(act, ParamPinSpec(this, unit));
			}
		}
	}
	return NULL;
}

void MachineItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* ev)
{
	QList<MachineItem*> sel = m_editor->getSelectedMachineItems();
	if (sel.length() == 1 && sel[0] == this)
	{
		QMenu menu;

		QMap<QAction*, Parameter::ParamPinSpec> actParamPin;
		QMenu* parampinmenu = m_mac->m_params->populateParamPinMenu(&menu, actParamPin);
		parampinmenu->setTitle(tr("&Parameter pins"));

		if (actParamPin.isEmpty())
		{
			delete parampinmenu;
			parampinmenu = NULL;
		}

		menu.addSeparator();
		QAction* actRename = menu.addAction(tr("&Name and appearance"));
		QAction* actDelete = menu.addAction(tr("&Delete"));

		QAction* action = menu.exec(ev->screenPos());
		if (action == actRename)
		{
			Dlg_MachineRename dlg(m_mac, m_editor);
			if (dlg.exec() == QDialog::Accepted)
			{
				QString newname = dlg.getName();
				MachineTypeHint::e newcolorhint = dlg.getColorType();

				if (newname != m_mac->getName() || newcolorhint != m_mac->getColorHint())
				{
					theUndo().push(new RenameMachineCommand(m_mac, newname, newcolorhint));
				}
			}
		}
		else if (action == actDelete)
		{
			theUndo().push(new DeleteMachineCommand(m_editor->getRouting(), m_mac));
		}
		else if (actParamPin.contains(action))
		{
			const Parameter::ParamPinSpec& pps = actParamPin.value(action);
			if (pps.param->getParamPin())
			{
				theUndo().push(new RemoveParamPinCommand(m_mac, pps.param));
			}
			else
			{
				theUndo().push(new AddParamPinCommand(m_mac, pps.param, pps.timeUnit));
			}
		}
	}
	else if (sel.length() > 1)
	{
	}
}

void MachineItem::onMachinePosChanged()
{
	if (pos() != m_mac->m_pos)
		setPos(m_mac->m_pos);
}

////////////////////////////////////////////////////////////////////////////

PinItem::PinItem(Editor* editor, const Ptr<Pin>& pin, MachineItem* parent)
: m_pin(pin), QGraphicsPathItem(parent), m_editor(editor), m_newConnectionItem(NULL)
{
	m_mouseMode = none;

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
