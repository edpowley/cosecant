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
	m_scanCodeOctaveUp = 55; // numpad *
	m_scanCodeOctaveDown = 309; // numpad /

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
	else if (scan == m_scanCodeOctaveUp)
	{
		if (m_octave < c_maxOctave) setOctave(m_octave+1);
		return true;
	}
	else if (scan == m_scanCodeOctaveDown)
	{
		if (m_octave > 0) setOctave(m_octave-1);
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
    KeyJazzEvent ev(type, note);
    sendEvent(ev);
}

//////////////////////////////////////////////////////////////////////////////////

QString KeyJazz::getKeyNameForNote(double note)
{
	QStringList names;
	foreach(quint32 scancode, m_noteScanCodes.keys(note - m_octave*12.0 - m_transpose))
	{
		UINT vk = MapVirtualKey(scancode, MAPVK_VSC_TO_VK);
		if (vk == 0) continue;
		UINT ch = MapVirtualKey(vk, MAPVK_VK_TO_CHAR);
		if (ch == 0) continue;
		names.append(QString(QChar(ch)));
	}
	return names.join(" ");
}

void KeyJazz::setOctave(int octave)
{
	octave = clamp(octave, 0, c_maxOctave);
	if (m_octave != octave)
	{
		m_octave = octave;
		signalChangeOctave(octave);
	}
}

void KeyJazz::setTranspose(int transpose)
{
	transpose = clamp(transpose, 0, 11);
	if (m_transpose != transpose)
	{
		m_transpose = transpose;
		signalChangeTranspose(transpose);
	}
}

