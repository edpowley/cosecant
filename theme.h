#pragma once

#include "cosecant_api.h"

class Theme : public Object
{
protected:
	Theme();
	static SingletonPtr<Theme> s_singleton;

public:
	static void initSingleton();
	static Theme& get() { return *s_singleton; }

	QColor getSignalTypeColor(CosecantAPI::SignalType::e type);
	QColor getMachineTypeHintColor(CosecantAPI::MachineTypeHint::e type);

	QColor getColor(const QString& key);

protected:
	QColor m_missingColor;
	QHash<QString, QColor> m_colorMap;
};
