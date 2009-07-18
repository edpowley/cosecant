#include "stdafx.h"
#include "common.h"
#include "audioio.h"
#include "workqueue.h"
#include "perfclock.h"

PrefsVar_String	AudioIO::s_prefInDevice ("audio/devices/i/name", "");
PrefsVar_String	AudioIO::s_prefOutDevice("audio/devices/o/name", "");
PrefsVar_Int AudioIO::s_prefInDeviceApi ("audio/devices/i/api", -1);
PrefsVar_Int AudioIO::s_prefOutDeviceApi("audio/devices/o/api", -1);

SingletonPtr<AudioIO> AudioIO::s_singleton;
__int64 AudioIO::s_perfCount;

void AudioIO::initSingleton()
{
	s_singleton.set(new AudioIO());
}

int paDo(PaError retcode)
{
	if (retcode < 0)
	{
		throw AudioIO::Error(Pa_GetErrorText(retcode));
	}
	return retcode;
}

AudioIO::AudioIO()
:	m_stream(NULL)
{
	paDo(Pa_Initialize());

	if (s_prefOutDeviceApi() == -1)
	{
		const PaDeviceInfo* dev = Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice());
		if (dev)
		{
			const PaHostApiInfo* api = Pa_GetHostApiInfo(dev->hostApi);
			s_prefOutDevice.set(dev->name);
			s_prefOutDeviceApi.set(api->type);
		}
	}

	m_inDeviceIndex  = findDevice(s_prefInDevice(),  s_prefInDeviceApi(),  input);
	m_outDeviceIndex = findDevice(s_prefOutDevice(), s_prefOutDeviceApi(), output);

	m_timeinfo.beatsPerSecond = 120.0 / 60.0;
	m_timeinfo.beatsPerBar = 4;
	m_timeinfo.beatsPerWholeNote = 4;
	m_timeinfo.barsPerSmallGrid = 4;
	m_timeinfo.smallGridsPerLargeGrid = 4;
	m_timeinfo.samplesPerSecond = 44100;
}

AudioIO::~AudioIO()
{
	if (m_stream) paDo(Pa_CloseStream(m_stream));
	paDo(Pa_Terminate());
}

void AudioIO::setDeviceIndexes(PaDeviceIndex inIndex, PaDeviceIndex outIndex)
{
	m_inDeviceIndex  = inIndex;
	m_outDeviceIndex = outIndex;
	setDevicePref(inIndex,  s_prefInDevice,  s_prefInDeviceApi);
	setDevicePref(outIndex, s_prefOutDevice, s_prefOutDeviceApi);
}

void AudioIO::setDevicePref(PaDeviceIndex index, PrefsVar_String& pref, PrefsVar_Int& prefApi)
{
	if (index == paNoDevice)
	{
		pref.set("");
		prefApi.set(-1);
	}
	else
	{
		const PaDeviceInfo* dev = Pa_GetDeviceInfo(index);
		ASSERT(dev);
		const PaHostApiInfo* api = Pa_GetHostApiInfo(dev->hostApi);
		ASSERT(api);

		pref.set(dev->name);
		prefApi.set(api->type);
	}
}

int AudioIO::paCallback(const void* inbuf, void* outbuf, unsigned long frames,
						const PaStreamCallbackTimeInfo* time, 
						PaStreamCallbackFlags status,
						void* user)
{
	boost::shared_lock<boost::shared_mutex> wqlock(WorkQueue::s_mutex);

	AudioIO& s = *s_singleton;

	PerfClockAutoCount clock(&s_perfCount);

	s.m_inbuf = (float*)inbuf;

	// Clear output buffers
	s.m_outbuf = (float*)outbuf;
	for (unsigned long i=0; i<frames * s.m_numOutputChannels; i++)
	{
		s.m_outbuf[i] = 0.0f;
	}

	// Check for a current workqueue
	WorkQueue* wq  = WorkQueue::s;
	if (!wq) return paContinue;

	// Work
	boost::upgrade_lock<boost::shared_mutex> playPosLock(wq->m_sequence->m_playPosMutex);
	bool seqPlaying			= wq->m_sequence->m_playing;
	wq->m_playing			= seqPlaying;
	double playPos			= wq->m_sequence->m_playPos;
	double loopStart		= wq->m_sequence->m_loopStart;
	double loopEnd			= wq->m_sequence->m_loopEnd;
	double ticksPerFrame	= wq->m_sequence->m_ticksPerFrame;
	wq->m_ticksPerFrame		= ticksPerFrame;
	double framesPerTick	= 1.0 / ticksPerFrame;
	wq->m_framesPerTick		= framesPerTick;

	for (unsigned long firstframe = 0; firstframe < frames; /* firstframe incremented at end of loop */)
	{
		int numFramesForThisIter = min<int>(c_maxBufferSize, frames - firstframe);
		if (seqPlaying)
		{
			int framesUntilLoopEnd = (int)ceil((loopEnd - playPos) * framesPerTick);
			numFramesForThisIter = min(numFramesForThisIter, framesUntilLoopEnd);
		}

		wq->reset();
		wq->m_playPos = playPos;

		while (!wq->finished())
		{
			WorkUnit::Base* wu;
			{
				boost::unique_lock<boost::shared_mutex> lock(wq->m_mutex);
				wu = wq->popReady();
			}

			// Until I implement multi-threading, wu should ALWAYS be non-null here
			wu->go(numFramesForThisIter);
		}

		if (s.m_inbuf)
			s.m_inbuf += numFramesForThisIter * s.m_numInputChannels;

		s.m_outbuf += numFramesForThisIter * s.m_numOutputChannels;

		if (seqPlaying)
		{
			playPos += ticksPerFrame * numFramesForThisIter;
			if (playPos > loopEnd)
			{
				wq->m_shouldUpdateSequenceFromScratch = true;
				playPos = loopStart;
			}
			else
			{
				wq->m_shouldUpdateSequenceFromScratch = false;
			}
		}

		firstframe += numFramesForThisIter;
	}

	boost::upgrade_to_unique_lock<boost::shared_mutex> playPosLockUpgrade(playPosLock);
	wq->m_sequence->m_playPos = playPos;

	return paContinue;
}

void AudioIO::start()
{
	if (m_stream) paDo(Pa_StartStream(m_stream));
}

void AudioIO::stop()
{
	if (m_stream) paDo(Pa_StopStream(m_stream));
}

void AudioIO::abort()
{
	if (m_stream) paDo(Pa_AbortStream(m_stream));
}

void AudioIO::open()
{
	PaDeviceIndex indevice  = getInDeviceIndex();
	PaDeviceIndex outdevice = getOutDeviceIndex();

	if (outdevice == paNoDevice)
	{
		throw Error(tr("No output device set. One possible cause for this is if you have recently changed soundcards, "
			"and the card specified in Cosecant's settings file is no longer present."));
	}

	PaStreamParameters spI, spO;
	memset(&spI, 0, sizeof(PaStreamParameters));
	memset(&spO, 0, sizeof(PaStreamParameters));

	m_numInputChannels = m_numOutputChannels = 0;

	if (indevice != paNoDevice)
	{
		m_numInputChannels = spI.channelCount = Pa_GetDeviceInfo(indevice)->maxInputChannels;
		spI.device = indevice;
		spI.hostApiSpecificStreamInfo = NULL;
		spI.sampleFormat = paFloat32;
		spI.suggestedLatency = Pa_GetDeviceInfo(indevice)->defaultLowInputLatency;
	}

	m_numOutputChannels = spO.channelCount = Pa_GetDeviceInfo(outdevice)->maxOutputChannels;
	spO.device = outdevice;
	spO.hostApiSpecificStreamInfo = NULL;
	spO.sampleFormat = paFloat32;
	spO.suggestedLatency = Pa_GetDeviceInfo(outdevice)->defaultLowOutputLatency;

	paDo(Pa_OpenStream(&m_stream,
		(indevice != paNoDevice) ? &spI : NULL,
		&spO,
		44100,
		paFramesPerBufferUnspecified,
		paNoFlag,
		&paCallback,
		(void*)this));

	signalOpen();
}

void AudioIO::close()
{
	if (m_stream) paDo(Pa_CloseStream(m_stream));
	m_stream = NULL;
}

PaDeviceIndex AudioIO::findDevice(const QString& name, int apiid, FindDeviceType devtype)
{
	if (apiid == -1) return paNoDevice;

	int numDevices = Pa_GetDeviceCount();
	if (numDevices < 0) return paNoDevice;

	for (int i=0; i<numDevices; i++)
	{
		const PaDeviceInfo* dev = Pa_GetDeviceInfo(i);
		const PaHostApiInfo* api = Pa_GetHostApiInfo(dev->hostApi);

		if (devtype == input  && dev->maxInputChannels  <= 0) continue;
		if (devtype == output && dev->maxOutputChannels <= 0) continue;

		if (api->type == apiid && dev->name == name)
			return i;
	}

	return paNoDevice;
}

QString AudioIO::getInputChannelName(int index)
{
	const PaDeviceInfo* dinfo = Pa_GetDeviceInfo(getInDeviceIndex());
	if (!dinfo) return "Error";
	const PaHostApiInfo* hinfo = Pa_GetHostApiInfo(dinfo->hostApi);
	if (!hinfo) return "Error";

	if (hinfo->type == paASIO)
	{
		const char* name = NULL;
		PaAsio_GetInputChannelName(getInDeviceIndex(), index, &name);
		return name;
	}
	else
	{
		return QString("Channel %1").arg(index);
	}
}

bool AudioIO::isOutputDeviceAsio(int index)
{
	const PaDeviceInfo* dinfo = Pa_GetDeviceInfo(index);
	if (!dinfo) return false;
	const PaHostApiInfo* hinfo = Pa_GetHostApiInfo(dinfo->hostApi);
	if (!hinfo) return false;

	return hinfo->type == paASIO;
}

QString AudioIO::getOutputChannelName(int index)
{
	if (isOutputDeviceAsio(getOutDeviceIndex()))
	{
		const char* name = NULL;
		PaAsio_GetOutputChannelName(getOutDeviceIndex(), index, &name);
		return name;
	}
	else
	{
		switch (index)
		{
		case 0: return "Channel 0 (front left)";
		case 1: return "Channel 1 (front right)";
		case 2: return "Channel 2 (front center)";
		case 3: return "Channel 3 (low frequency)";
		case 4: return "Channel 4 (back left)";
		case 5: return "Channel 5 (back right)";
		case 6: return "Channel 6 (side left)";
		case 7: return "Channel 7 (side right)";
		default: return QString("Channel %1").arg(index);
		}
	}
}
