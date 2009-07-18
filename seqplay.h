#pragma once

#include "cosecant_api.h"

class SeqPlay : public QObject
{
	Q_OBJECT

protected:
	SeqPlay();

	static SingletonPtr<SeqPlay> s_singleton;

public:
	static void initSingleton() { s_singleton.set(new SeqPlay); }
	static SeqPlay& get() { return *s_singleton; }

	const CosecantAPI::TimeInfo& getTimeInfo() { return m_timeinfo; }

protected:
	CosecantAPI::TimeInfo m_timeinfo;
	

};
