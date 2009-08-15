#include "stdafx.h"
#include "common.h"
#include "routingeditor.h"
using namespace RoutingEditor;

#include "undo_command_ids.h"
#include "song.h"
#include "machine.h"
#include "parameter.h"
#include "parameditor.h"
#include "dlg_machinerename.h"

/* TRANSLATOR RoutingEditor::Editor */

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

///////////////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////////////

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
	ev->accept();

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
