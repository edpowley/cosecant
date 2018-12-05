#pragma once

#include "object.h"
#include "ui_machine_rename_dlg.h"
#include "cosecant_api.h"

class Machine;

class Dlg_MachineRename : public QDialog
{
	Q_OBJECT

public:
	Dlg_MachineRename(const Ptr<Machine>& mac, QWidget *parent = 0, Qt::WindowFlags flags = 0);
	~Dlg_MachineRename();

	QString getName() { return ui.editName->text(); }
	CosecantAPI::MachineTypeHint::e getColorType();

protected:
	Ptr<Machine> m_mac;
	void addColorComboItem(CosecantAPI::MachineTypeHint::e type, const QString& text);
	QIcon createColorIcon(const QColor& color, int size = 16);

private:
	Ui::MachineRenameDlg ui;
};
