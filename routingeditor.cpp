#include "stdafx.h"
#include "common.h"
#include "routingeditor.h"
using namespace RoutingEditor;

#include "song.h"
#include "machinechooserwidget.h"

/* TRANSLATOR RoutingEditor::Editor */

PrefsVar_Double Editor::s_prefPinSize("routingeditor/pinsize", 6);
PrefsVar_Double Editor::s_prefConnBezierOffset("routingeditor/connbezieroffset", 50);
MachineChooserWidget* Editor::s_palette = NULL;

Editor::Editor(const Ptr<Routing>& routing, QWidget* parent)
: m_routing(routing), MyGraphicsView(parent), m_scene(this)
{
	setScene(&m_scene);
	setDragMode(RubberBandDrag);

	m_scene.setSceneRect(0,0,1000,1000);

	if (!s_palette)
		s_palette = new MachineChooserWidget;

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
