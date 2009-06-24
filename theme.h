#pragma once

#include "cosecant_api.h"

class Theme : public Object
{
protected:
	Theme() {}
	static SingletonPtr<Theme> s_singleton;

public:
	static void initSingleton();
	static Theme& get() { return *s_singleton; }

	QColor getSignalTypeColor(CosecantAPI::SignalType::st type);
	QColor getMachineTypeHintColor(CosecantAPI::MachineTypeHint::mt type);
};
