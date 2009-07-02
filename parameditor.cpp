#include "stdafx.h"
#include "common.h"
#include "parameditor.h"

ParamEditor::ParamEditor(const Ptr<Machine>& mac, QWidget* parent)
: QScrollArea(parent), m_mac(mac)
{
	QWidget* w = new QWidget(this);
	QGridLayout* grid = new QGridLayout(w);
	for (int i=0; i<50; i++)
		grid->addWidget(new QLabel(QString("Hello %1").arg(i), w), i,0);

	setWidget(w);
}
