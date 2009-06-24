#include "stdafx.h"
#include "common.h"
#include "cosecantmainwindow.h"

#include "song.h"
#include "routingeditor.h"

CosecantMainWindow::CosecantMainWindow(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);

	QAction* undoaction = Song::get().m_undo.createUndoAction(this);
	undoaction->setShortcut(tr("Ctrl+Z"));
	ui.menu_Edit->insertAction(ui.actionUndoredo_placeholder, undoaction);

	QAction* redoaction = Song::get().m_undo.createRedoAction(this);
	QList<QKeySequence> redoshortcuts;
	redoshortcuts << tr("Ctrl+Y") << tr("Shift+Ctrl+Z");
	redoaction->setShortcuts(redoshortcuts);
	ui.menu_Edit->insertAction(ui.actionUndoredo_placeholder, redoaction);

	ui.actionUndoredo_placeholder->setVisible(false);

	m_tabWidget = new QTabWidget();
	setCentralWidget(m_tabWidget);
	m_tabWidget->setMovable(true);
	m_tabWidget->setTabsClosable(true);

	RoutingEditor::Editor* re = new RoutingEditor::Editor(Song::get().m_routing, m_tabWidget);
	m_tabWidget->addTab(re, "Routing");

	m_tabWidget->setDocumentMode(true);
}

CosecantMainWindow::~CosecantMainWindow()
{

}
