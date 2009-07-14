#ifndef COSECANTMAINWINDOW_H
#define COSECANTMAINWINDOW_H

#include "ui_cosecantmainwindow.h"
#include "prefs.h"

class CosecantMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	static CosecantMainWindow* get() { return s_singleton; }

	CosecantMainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~CosecantMainWindow();

protected slots:
	// Auto connected slots
	void on_actionSettings_triggered();

private:
	static CosecantMainWindow* s_singleton;

	Ui::CosecantMainWindowClass ui;

	QTabWidget* m_tabWidget;
};

#endif // COSECANTMAINWINDOW_H
