#pragma once

#include "prefs.h"
#include "cosecant_api.h"

class AudioIO : public QObject
{
	Q_OBJECT

protected:
	AudioIO();

	static SingletonPtr<AudioIO> s_singleton;

public:
	~AudioIO();
	static void initSingleton();
	static AudioIO& get() { return *s_singleton; }

	static PrefsVar_Int s_prefInDeviceIndex, s_prefOutDeviceIndex;

	void start();
	void stop();
	void abort();

	PaError open();
	void close();

	static const unsigned int c_maxBufferSize = CosecantAPI::maxFramesPerBuffer;

	int m_numInputChannels, m_numOutputChannels;
	QString getInputChannelName(int index);
	QString getOutputChannelName(int index);
	bool isOutputDeviceAsio(int index);

	float* m_inbuf;
	boost::mutex m_outmutex;
	float* m_outbuf;

	static __int64 s_perfCount;

signals:
	void signalOpen();

protected:
	PaStream* m_stream;
	static int paCallback(const void* inbuf, void* outbuf, unsigned long frames,
		const PaStreamCallbackTimeInfo* time, PaStreamCallbackFlags status, void* user);
};
