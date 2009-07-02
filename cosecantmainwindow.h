#ifndef COSECANTMAINWINDOW_H
#define COSECANTMAINWINDOW_H

#include "ui_cosecantmainwindow.h"

class CosecantMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	static CosecantMainWindow* get() { return s_singleton; }

	CosecantMainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~CosecantMainWindow();

private:
	static CosecantMainWindow* s_singleton;

	Ui::CosecantMainWindowClass ui;

	QTabWidget* m_tabWidget;
};

#endif // COSECANTMAINWINDOW_H
