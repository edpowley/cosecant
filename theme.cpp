#include "stdafx.h"
#include "common.h"
#include "theme.h"
using namespace CosecantAPI;

SingletonPtr<Theme> Theme::s_singleton;

void Theme::initSingleton()
{
	s_singleton.set(new Theme);
}

Theme::Theme() : m_missingColor(255,0,255)
{
	qDebug() << QColor::colorNames();
	m_colorMap.insert("SequenceEditor/Ruler/Bar",  QColor("light steel blue"));
	m_colorMap.insert("SequenceEditor/Ruler/Grid", QColor("steel blue"));
	m_colorMap.insert("SequenceEditor/Ruler/Odd",  QColor("white"));
	m_colorMap.insert("SequenceEditor/Ruler/Even", QColor("azure"));
}

QColor Theme::getSignalTypeColor(SignalType::st type)
{
	switch (type)
	{
	case SignalType::monoAudio:		return QColor("blue");
	case SignalType::stereoAudio:	return QColor("black");
	case SignalType::paramControl:	return QColor("green");
	case SignalType::noteTrigger:	return QColor("orange");
	default:						return m_missingColor;
	}
}

QColor Theme::getMachineTypeHintColor(MachineTypeHint::mt type)
{
	switch (type)
	{
	case MachineTypeHint::master:		return QColor("dark khaki");
	case MachineTypeHint::generator:	return QColor("cornflower blue");
	case MachineTypeHint::effect:		return QColor("indian red");
	case MachineTypeHint::control:		return QColor("medium sea green");
	case MachineTypeHint::none:
	default:							return m_missingColor;
	}
}

QColor Theme::getColor(const QString& key)
{
	return m_colorMap.value(key, m_missingColor);
}
