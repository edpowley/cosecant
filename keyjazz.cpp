#include "stdafx.h"
#include "common.h"
#include "keyjazz.h"
#include "application.h"

SingletonPtr<KeyJazz> KeyJazz::s_singleton;
QEvent::Type KeyJazzEvent::s_type = QEvent::None;

KeyJazz::KeyJazz()
: m_octave(4), m_transpose(0)
{
	KeyJazzEvent::s_type = static_cast<QEvent::Type>(QEvent::registerEventType());

	m_scanCodeOff = 2; // 1
	m_scanCodeExtend = 30; // A

	// ZXC row
	{
		quint32 sc[] = {44,31,45,32,46,47,34,48,35,49,36,50,51,0};
		addScanCodes(sc, 0);
	}

	// QWE row
	{
		quint32 sc[] = {16,3,17,4,18,19,6,20,7,21,8,22,23,10,24,11,25,26,13,27,0};
		addScanCodes(sc, 12);
	}
}

void KeyJazz::addScanCodes(quint32* sc, double note)
{
	for (; *sc; ++sc, ++note)
		m_noteScanCodes.insert(*sc, note);
}

bool KeyJazz::keyPress(QKeyEvent* ev)
{
	quint32 scan = ev->nativeScanCode();
	if (scan == m_scanCodeOff)
	{
		sendEvent(KeyJazzEvent::off);
		return true;
	}
	else if (scan == m_scanCodeExtend)
	{
		sendEvent(KeyJazzEvent::extend);
		return true;
	}
	else if (m_noteScanCodes.contains(scan))
	{
		double note = m_noteScanCodes.value(scan) + m_octave*12.0 + m_transpose;
		sendEvent(KeyJazzEvent::noteOn, note);
		return true;
	}

	return false;
}

void KeyJazz::postEvent(KeyJazzEvent* ev)
{
	QWidget* w = Application::get().focusWidget();
	if (w) Application::get().postEvent(w, ev);
}

void KeyJazz::postEvent(KeyJazzEvent::Type type, double note)
{
	postEvent(new KeyJazzEvent(type, note));
}

void KeyJazz::sendEvent(KeyJazzEvent& ev)
{
	QWidget* w = Application::get().focusWidget();
	if (w) Application::get().sendEvent(w, &ev);
}

void KeyJazz::sendEvent(KeyJazzEvent::Type type, double note)
{
	sendEvent(KeyJazzEvent(type, note));
}
