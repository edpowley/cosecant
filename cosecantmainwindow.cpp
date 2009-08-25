#include "stdafx.h"
#include "common.h"
#include "cosecantmainwindow.h"
#include "version.h"

#include "song.h"
#include "seqplay.h"
#include "routingeditor.h"
#include "sequenceeditor.h"
#include "dlg_settings.h"
#include "dlg_about.h"
#include "keyjazz.h"

CosecantMainWindow* CosecantMainWindow::s_singleton = NULL;

CosecantMainWindow::CosecantMainWindow(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags), m_currentTab(NULL)
{
	ui.setupUi(this);

	ASSERT(s_singleton == NULL);
	s_singleton = this;

	setWindowTitle(tr("BTDSys Cosecant %1").arg(getVersionString()));

	QAction* undoaction = theUndo().createUndoAction(this);
	undoaction->setShortcut(tr("Ctrl+Z", "shortcut for edit/undo"));
	undoaction->setIcon(QIcon(":/CosecantMainWindow/images/edit-undo.png"));
	ui.menu_Edit->insertAction(ui.actionUndoredo_placeholder, undoaction);
	ui.mainToolBar->insertAction(ui.actionUndoredo_placeholder, undoaction);

	QAction* redoaction = theUndo().createRedoAction(this);
	QList<QKeySequence> redoshortcuts;
	redoshortcuts << tr("Ctrl+Y", "shortcut for edit/redo") << tr("Shift+Ctrl+Z", "shortcut for edit/redo");
	redoaction->setShortcuts(redoshortcuts);
	redoaction->setIcon(QIcon(":/CosecantMainWindow/images/edit-redo.png"));
	ui.menu_Edit->insertAction(ui.actionUndoredo_placeholder, redoaction);
	ui.mainToolBar->insertAction(ui.actionUndoredo_placeholder, redoaction);

	ui.actionUndoredo_placeholder->setVisible(false);

	ui.keyjazzToolBar->addWidget(ui.widgetKeyJazzOctave);
	for (int i=0; i<=KeyJazz::c_maxOctave; i++)
		ui.comboKeyJazzOctave->addItem(QString::number(i));
	ui.comboKeyJazzOctave->setCurrentIndex(KeyJazz::get().getOctave());

	ui.keyjazzToolBar->addWidget(ui.widgetKeyJazzTranspose);
	static const char* notenames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
	for (int i=0; i<12; i++)
		ui.comboKeyJazzTranspose->addItem( QString("+%1 %2").arg(i).arg(notenames[i]) );
	ui.comboKeyJazzTranspose->setCurrentIndex(KeyJazz::get().getTranspose());

	connect(ui.comboKeyJazzOctave, SIGNAL(currentIndexChanged(int)),
		&KeyJazz::get(), SLOT(setOctave(int)) );
	connect(&KeyJazz::get(), SIGNAL(signalChangeOctave(int)),
		ui.comboKeyJazzOctave, SLOT(setCurrentIndex(int)) );

	connect(ui.comboKeyJazzTranspose, SIGNAL(currentIndexChanged(int)),
		&KeyJazz::get(), SLOT(setTranspose(int)) );
	connect(&KeyJazz::get(), SIGNAL(signalChangeTranspose(int)),
		ui.comboKeyJazzTranspose, SLOT(setCurrentIndex(int)) );

	// Must do this _before_ adding any tabs
	m_paletteDock = new QDockWidget(tr("Context palette"), this);
	m_paletteDock->setAllowedAreas(Qt::AllDockWidgetAreas);
	addDockWidget(Qt::LeftDockWidgetArea, m_paletteDock);
	QAction* palettedockaction = m_paletteDock->toggleViewAction();
	ui.menuWindow->addAction(palettedockaction);

	m_paletteEmpty = new QLabel(tr("<empty>"), this);
	m_paletteEmpty->setAlignment(Qt::AlignCenter);
	m_paletteEmpty->setWordWrap(true);
	m_paletteDock->setWidget(m_paletteEmpty);

	m_tabWidget = new QTabWidget();
	setCentralWidget(m_tabWidget);
	m_tabWidget->setMovable(true);
	m_tabWidget->setTabsClosable(true);

	addTab(new RoutingEditor::Editor(Song::get().m_routing, m_tabWidget));
	addTab(new SequenceEditor::Editor(Song::get().m_sequence, m_tabWidget));

	m_tabWidget->setDocumentMode(true);

	connect( m_tabWidget, SIGNAL(currentChanged(int)),
		this, SLOT(onTabChanged(int)) );

	m_tabWidget->setCurrentIndex(0);

	QDockWidget* undodock = new QDockWidget(tr("Undo history"), this);
	undodock->setAllowedAreas(Qt::AllDockWidgetAreas);
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

void CosecantMainWindow::addTab(MWTab* tab)
{
	QWidget* widget = tab->getMWTabWidget();
	m_tabWidget->addTab(widget, tab->getTitle());
	m_widgetTabs.insert(widget, tab);
	
	m_tabWidget->setCurrentWidget(widget);
}

void CosecantMainWindow::onTabChanged(int index)
{
	if (m_currentTab)
	{
		m_currentTab = NULL;
	}

	MWTab* tab = m_widgetTabs.value( m_tabWidget->widget(index), NULL );
	if (!tab) return;
	
	QWidget* palette = tab->getPalette();
	if (!palette) palette = m_paletteEmpty;
	m_paletteDock->setWidget(palette);
	palette->show();

	foreach(QAction* action, ui.contextToolBar->actions())
	{
		ui.contextToolBar->removeAction(action);
	}

	foreach(QAction* action, tab->getToolBarActions())
	{
		ui.contextToolBar->addAction(action);
	}

	m_currentTab = tab;
}

////////////////////////////////////////////////////////////////////////////////

void CosecantMainWindow::on_actionSettings_triggered()
{
	Dlg_Settings::run(this);
}

void CosecantMainWindow::on_actionAbout_triggered()
{
	Dlg_About dlg;
	dlg.exec();
}

void CosecantMainWindow::on_actionTransportRewind_triggered()
{
	qDebug() << "rewind";
}

void CosecantMainWindow::on_actionTransportPlay_toggled(bool checked)
{
	qDebug() << "play" << checked;

	if (!checked) ui.actionTransportRecord->setChecked(false);

	SeqPlay::get().setPlaying(checked);
}

void CosecantMainWindow::on_actionTransportStop_triggered()
{
	qDebug() << "stop";

	ui.actionTransportPlay->setChecked(false);
}

void CosecantMainWindow::on_actionTransportRecord_toggled(bool checked)
{
	qDebug() << "record" << checked;

	if (checked) ui.actionTransportPlay->setChecked(true);
}

//////////////////////////////////////////////////////////////////////////////

void CosecantMainWindow::on_actionFileOpen_triggered()
{
	qDebug() << "open";

	try
	{
		Song::get().load("aaaaa_testsong.zip");
	}
	catch (const SongLoadError& err)
	{
		QMessageBox::critical(this, QString(), err.msg());
	}
}

void CosecantMainWindow::on_actionFileSave_triggered()
{
	qDebug() << "save";

	try
	{
		Song::get().save("aaaaa_testsong.zip");
	}
	catch (const SongSaveError& err)
	{
		QMessageBox::critical(this, QString(), err.msg());
	}
}
