#include "stdafx.h"
#include "common.h"
#include "dlg_machinerename.h"
#include "theme.h"
#include "machine.h"

#include "cosecant_api.h"
using namespace CosecantAPI;

Dlg_MachineRename::Dlg_MachineRename(const Ptr<Machine>& mac, QWidget *parent, Qt::WFlags flags)
: QDialog(parent, flags), m_mac(mac)
{
	ui.setupUi(this);

	addColorComboItem(MachineTypeHint::master,		tr("Master"));
	addColorComboItem(MachineTypeHint::generator,	tr("Generator"));
	addColorComboItem(MachineTypeHint::effect,		tr("Effect"));
	addColorComboItem(MachineTypeHint::control,		tr("Control"));

	int index = ui.comboColor->findData(QVariant::fromValue<unsigned int>(m_mac->getColorHint()));
	if (index != -1) ui.comboColor->setCurrentIndex(index);

	ui.editName->setText(m_mac->getName());
	ui.editName->setFocus();
	ui.editName->selectAll();
}

void Dlg_MachineRename::addColorComboItem(MachineTypeHint::mt type, const QString& text)
{
	ui.comboColor->addItem(
		createColorIcon( Theme::get().getMachineTypeHintColor(type) ),
		text,
		QVariant::fromValue<unsigned int>(type)
	);
}

MachineTypeHint::mt Dlg_MachineRename::getColorType()
{
	int index = ui.comboColor->currentIndex();
	if (index != -1)
	{
		return static_cast<MachineTypeHint::mt>(ui.comboColor->itemData(index).value<unsigned int>());
	}
	
	return MachineTypeHint::none;
}

Dlg_MachineRename::~Dlg_MachineRename()
{
}

QIcon Dlg_MachineRename::createColorIcon(const QColor& color, int size)
{
	QPixmap pixmap(size,size);
	pixmap.fill(color);

	{
		QPainter painter(&pixmap);
		painter.setPen(Qt::black);
		painter.drawRect(0, 0, size-1, size-1);
	}

	return QIcon(pixmap);
}
