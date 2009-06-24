#include "stdafx.h"
#include "common.h"
#include "cosecantmainwindow.h"

#include "song.h"
#include "routingeditor.h"

CosecantMainWindow::CosecantMainWindow(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);

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
