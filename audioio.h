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
	ERROR_CLASS(Error);

	~AudioIO();
	static void initSingleton();
	static AudioIO& get() { return *s_singleton; }

	PaDeviceIndex getInDeviceIndex()  { return m_inDeviceIndex; }
	PaDeviceIndex getOutDeviceIndex() { return m_outDeviceIndex; }
	void setDeviceIndexes(PaDeviceIndex inIndex, PaDeviceIndex outIndex);

	void start();
	void stop();
	void abort();

	void open();
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

	enum FindDeviceType {input, output};
	PaDeviceIndex findDevice(const QString& name, int apiid, FindDeviceType devtype);

	static PrefsVar_String s_prefInDevice, s_prefOutDevice;
	static PrefsVar_Int s_prefInDeviceApi, s_prefOutDeviceApi;
	PaDeviceIndex m_inDeviceIndex, m_outDeviceIndex;
	void setDevicePref(PaDeviceIndex index, PrefsVar_String& pref, PrefsVar_Int& prefApi);
};
