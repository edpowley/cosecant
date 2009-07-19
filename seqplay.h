#pragma once

#include "cosecant_api.h"

class SeqPlay : public QObject
{
	Q_OBJECT

protected:
	SeqPlay();

	static SingletonPtr<SeqPlay> s_singleton;

public:
	boost::shared_mutex m_mutex;

	static void initSingleton() { s_singleton.set(new SeqPlay); }
	static SeqPlay& get() { return *s_singleton; }

	const CosecantAPI::TimeInfo& getTimeInfo() { return m_timeinfo; }

	bool isPlaying() { return m_playing; }
	double getPlayPos() { return m_playPos; }

	void setPlaying(bool playing);

protected:
	CosecantAPI::TimeInfo m_timeinfo;
	
	bool m_playing;
	double m_playPos; // in beats
};
