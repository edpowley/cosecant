#include "stdafx.h"
#include "common.h"
#include "cosecantmainwindow.h"

#include "song.h"
#include "routingeditor.h"

CosecantMainWindow::CosecantMainWindow(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);

	QAction* undoaction = theUndo().createUndoAction(this);
	undoaction->setShortcut(tr("Ctrl+Z"));
	ui.menu_Edit->insertAction(ui.actionUndoredo_placeholder, undoaction);

	QAction* redoaction = theUndo().createRedoAction(this);
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

	QDockWidget* undodock = new QDockWidget(tr("Undo stack"), this);
	undodock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	QUndoView* undoview = new QUndoView(&theUndo(), undodock);
	undodock->setWidget(undoview);
	addDockWidget(Qt::LeftDockWidgetArea, undodock);
	undodock->hide();
	QAction* undodockaction = undodock->toggleViewAction();
	ui.menuWindow->addAction(undodockaction);
}

CosecantMainWindow::~CosecantMainWindow()
{

}
