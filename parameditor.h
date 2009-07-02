#pragma once

#include "machine.h"

class ParamEditor : public QScrollArea
{
	Q_OBJECT

public:
	ParamEditor(const Ptr<Machine>& mac, QWidget* parent);

protected:
	Ptr<Machine> m_mac;
};
