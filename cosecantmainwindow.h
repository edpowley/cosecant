#ifndef COSECANTMAINWINDOW_H
#define COSECANTMAINWINDOW_H

#include "ui_cosecantmainwindow.h"

class CosecantMainWindow : public QMainWindow
{
	Q_OBJECT

public:
	CosecantMainWindow(QWidget *parent = 0, Qt::WFlags flags = 0);
	~CosecantMainWindow();

private:
	Ui::CosecantMainWindowClass ui;

	QTabWidget* m_tabWidget;
};

#endif // COSECANTMAINWINDOW_H
