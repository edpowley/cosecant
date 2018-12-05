#pragma once

#include "ui_about_dlg.h"

class Dlg_About : public QDialog
{
	Q_OBJECT

public:
	Dlg_About(QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~Dlg_About();

protected:

private:
	Ui::AboutDlg ui;
};
