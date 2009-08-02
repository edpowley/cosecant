#pragma once

#include "builtinmachines.h"
#include "audioio.h"

namespace Builtin
{

///////////////////////////////////////////////////////////////////////

class AudioInOut : public BuiltinMachine
{
	Q_OBJECT

public:
	virtual void changeParam(ParamTag tag, double value)
	{
		qDebug() << "changeParam" << tag << value;
		if (tag == COSECANT_TAG('chan'))
		{
			int c = static_cast<int>(value) - 1;
			m_leftChannel  = c*2;
			m_rightChannel = c*2+1;
		}
	}

protected:
	int m_leftChannel, m_rightChannel;

	AudioInOut() : m_leftChannel(-1), m_rightChannel(-1)
	{
	}

	virtual int getNumChannels() = 0;

	static const EnumParamInfo& getChannelParamInfo()
	{
		static EnumParamInfo info;
		static bool initialised = false;
		if (!initialised)
		{
			info.p.tag = COSECANT_TAG('chan');
			info.p.name = "Channel";
			static const char* items[] = {"None", "Channel 1/2", NULL};
			info.items = items;
			info.def = 1;
			initialised = true;
		}
		return info;
	}

	static QStringList getChannelParamItems(int nChans, boost::function<QString(int)> getName)
	{
		QStringList items("None");
		for (int i=0; i<nChans/2; i++)
		{
			items << QString("%1, %2").arg(getName(i*2), getName(i*2+1));
		}
		return items;
	}

	void populateChannelParam(int nChans, boost::function<QString(int)> getName)
	{
		Ptr<Parameter::Enum> param = dynamic_cast<Parameter::Enum*>(
			m_paramMap.get(COSECANT_TAG('chan')).c_ptr() );
		param->setItems(getChannelParamItems(nChans, getName));
	}
};

////////////////////////////////////////////////////////////////////////////////

class AudioOut : public AudioInOut
{
	Q_OBJECT

public:
	AudioOut() : AudioInOut() {}

	virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe)
	{
		int nChans = AudioIO::get().m_numOutputChannels;
		if (m_leftChannel  < 0 || m_leftChannel  >= nChans) return;
		if (m_rightChannel < 0 || m_rightChannel >= nChans) return;

		QMutexLocker lock(&AudioIO::get().m_outmutex);

		for (int i=firstframe; i<lastframe; i++)
		{
			AudioIO::get().m_outbuf[i * nChans + m_leftChannel ] += inpins[0].f[i*2];
			AudioIO::get().m_outbuf[i * nChans + m_rightChannel] += inpins[0].f[i*2+1];
		}
	}

	virtual void initInfo()
	{
		static MachineInfo info;

		static bool initialised = false;
		if (!initialised)
		{
			info.defaultName = "AOut";
			info.typeHint = MachineTypeHint::master;
			
			static const ParamInfo* params[] = { &getChannelParamInfo().p, NULL };
			info.params.params = params;

			static PinInfo pin = { "To soundcard", SignalType::stereoAudio };
			static const PinInfo* pins[] = { &pin, NULL };
			info.inPins = pins;

			initialised = true;
		}

		m_info = &info;
	}

	void populateChannelParam()
	{
		AudioInOut::populateChannelParam(AudioIO::get().m_numOutputChannels,
			boost::bind(&AudioIO::getOutputChannelName, &AudioIO::get(), _1));
	}

protected:
	virtual void initImpl() { populateChannelParam(); }
	virtual int getNumChannels() { return AudioIO::get().m_numOutputChannels; }
};

////////////////////////////////////////////////////////////////////////////////

class AudioIn : public AudioInOut
{
	Q_OBJECT

public:
	AudioIn() : AudioInOut() {}

	virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe)
	{
		int nChans = AudioIO::get().m_numInputChannels;
		if (m_leftChannel  < 0 || m_leftChannel  >= nChans) return;
		if (m_rightChannel < 0 || m_rightChannel >= nChans) return;

		for (int i=firstframe; i<lastframe; i++)
		{
			outpins[0].f[i*2  ] = AudioIO::get().m_inbuf[i * nChans + m_leftChannel];
			outpins[0].f[i*2+1] = AudioIO::get().m_inbuf[i * nChans + m_rightChannel];
		}
	}

	virtual void initInfo()
	{
		static MachineInfo info;

		static bool initialised = false;
		if (!initialised)
		{
			info.defaultName = "AIn";
			info.typeHint = MachineTypeHint::master;
			
			static const ParamInfo* params[] = { &getChannelParamInfo().p, NULL };
			info.params.params = params;

			static PinInfo pin = { "From soundcard", SignalType::stereoAudio };
			static const PinInfo* pins[] = { &pin, NULL };
			info.outPins = pins;

			initialised = true;
		}

		m_info = &info;
	}

	void populateChannelParam()
	{
		AudioInOut::populateChannelParam(AudioIO::get().m_numInputChannels,
			boost::bind(&AudioIO::getInputChannelName, &AudioIO::get(), _1));
	}

protected:
	virtual void initImpl() { populateChannelParam(); }
	virtual int getNumChannels() { return AudioIO::get().m_numInputChannels; }
};

////////////////////////////////////////////////////////////////////////////////

}; // end namespace Builtin
