#ifndef COSECANTMAINWINDOW_H
#define COSECANTMAINWINDOW_H

#include "ui_cosecantmainwindow.h"
#include "prefs.h"
#include "mwtab.h"

class CosecantMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	static CosecantMainWindow* get() { return s_singleton; }

	CosecantMainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~CosecantMainWindow();

	void addTab(MWTab* tab);

protected slots:
	void onTabChanged(int index);

	// Auto connected slots
	void on_actionSettings_triggered();
	void on_actionAbout_triggered();

	void on_actionTransportRewind_triggered();
	void on_actionTransportPlay_toggled(bool checked);
	void on_actionTransportRecord_toggled(bool checked);
	void on_actionTransportStop_triggered();

private:
	static CosecantMainWindow* s_singleton;

	Ui::CosecantMainWindowClass ui;

	QTabWidget* m_tabWidget;
	QHash<QWidget*, MWTab*> m_widgetTabs;
	MWTab* m_currentTab;
};

#endif // COSECANTMAINWINDOW_H
