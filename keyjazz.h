#pragma once

class KeyJazzEvent : public QEvent
{
public:
	enum Type { noteOn, noteOff, off, extend };

	static QEvent::Type s_type;

	KeyJazzEvent(Type type, double note) : QEvent(s_type), m_type(type), m_note(note) { ignore(); }

	Type getType() { return m_type; }
	double getNote() { return m_note; }

protected:
	Type m_type;
	double m_note;
};

class KeyJazz
{
protected:
	static SingletonPtr<KeyJazz> s_singleton;
	KeyJazz();

public:
	static void initSingleton() { s_singleton.set(new KeyJazz); }
	static KeyJazz& get() { return *s_singleton; }

	bool keyPress(QKeyEvent* ev);

protected:
	QHash<quint32, double> m_noteScanCodes;
	quint32 m_scanCodeOff, m_scanCodeExtend;
	void addScanCodes(quint32* sc, double offset);

	void postEvent(KeyJazzEvent* ev);
	void postEvent(KeyJazzEvent::Type type, double note=0.0);
	void sendEvent(KeyJazzEvent& ev);
	void sendEvent(KeyJazzEvent::Type type, double note=0.0);

	double m_octave, m_transpose;
};
