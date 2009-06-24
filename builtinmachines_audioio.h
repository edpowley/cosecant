#pragma once

#include "builtinmachines.h"
#include "audioio.h"

namespace Builtin
{

///////////////////////////////////////////////////////////////////////

class AudioInOut : public Machine
{
	Q_OBJECT

public slots:
	void sanitiseChannelParamState()
	{
		if (m_paramStates['chan'] >= getNumChannels()/2 + 1)
		{
			boost::unique_lock<boost::mutex> lock(m_paramChangesMutex);
			m_paramChanges['chan'] = m_paramStates['chan'] = 0.0;
			// TODO fixme:
			//m_signalParamChange['chan']();
		}
	}

public:
	virtual void changeParam(ParamTag tag, ParamValue value)
	{
		if (tag == 'chan')
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
		connect(
			&AudioIO::get(), SIGNAL(signalOpen()),
			this, SLOT(sanitiseChannelParamState())
		);
	}

	virtual int getNumChannels() = 0;

	static void populateChannelParam(const Ptr<ParamInfo::Enum>& param, int nChans, boost::function<QString(int)> getName)
	{
		param->m_items.resize(1 + nChans/2);
		param->m_items[0] = "None";
		for (int i=0; i<nChans/2; i++)
		{
			param->m_items[i+1] = QString("%1\t / %2").arg(getName(i*2), getName(i*2+1));
		}
		param->itemsChanged();
	}
};

////////////////////////////////////////////////////////////////////////////////

class AudioOut : public AudioInOut
{
	Q_OBJECT

public:
	virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe)
	{
		int nChans = AudioIO::get().m_numOutputChannels;
		if (m_leftChannel  < 0 || m_leftChannel  >= nChans) return;
		if (m_rightChannel < 0 || m_rightChannel >= nChans) return;

		boost::unique_lock<boost::mutex> lock(AudioIO::get().m_outmutex);

		for (int i=firstframe; i<lastframe; i++)
		{
			AudioIO::get().m_outbuf[i * nChans + m_leftChannel ] += inpins[0].f[i*2];
			AudioIO::get().m_outbuf[i * nChans + m_rightChannel] += inpins[0].f[i*2+1];
		}
	}

	static Ptr<MachInfo> getInfo()
	{
		if (!s_channelParam)
		{
			s_channelParam = new ParamInfo::Enum("Channel", 'chan', std::vector<QString>(), 0);
			s_channelParam->m_def = 1;
			populateChannelParam();
			// TODO: fixme
/*			connect(
				&AudioIO::get(), SIGNAL(signalOpen()),
				????
			);
*/		}

		return (new MachInfo("builtin/aout"))
			->setName("AOut")
			->setTypeHint(MachineTypeHint::master)
			->addInPin(new PinInfo("To soundcard", SignalType::stereoAudio))
			->setParams((new ParamInfo::Group("",0))
				->add(s_channelParam)
			)
		;
	}

	static void populateChannelParam()
	{
		AudioInOut::populateChannelParam(s_channelParam, AudioIO::get().m_numOutputChannels,
			boost::bind(&AudioIO::getOutputChannelName, &AudioIO::get(), _1));
	}

protected:
	static Ptr<ParamInfo::Enum> s_channelParam;

	virtual int getNumChannels() { return AudioIO::get().m_numOutputChannels; }
};

////////////////////////////////////////////////////////////////////////////////

class AudioIn : public AudioInOut
{
	Q_OBJECT

public:
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

	static Ptr<MachInfo> getInfo()
	{
		if (!s_channelParam)
		{
			s_channelParam = new ParamInfo::Enum("Channel", 'chan', std::vector<QString>(), 0);
			s_channelParam->m_def = 1;
			populateChannelParam();
			// TODO: fixme
//			AudioIO::get().m_signalOpen.connect(sigc::ptr_fun(&AudioIn::populateChannelParam));
		}

		return (new MachInfo("builtin/ain"))
			->setName("AIn")
			->setTypeHint(MachineTypeHint::master)
			->addOutPin(new PinInfo("From soundcard", SignalType::stereoAudio))
			->setParams((new ParamInfo::Group("",0))
				->add(s_channelParam)
			)
		;
	}

	static void populateChannelParam()
	{
		AudioInOut::populateChannelParam(s_channelParam, AudioIO::get().m_numInputChannels,
			boost::bind(&AudioIO::getInputChannelName, &AudioIO::get(), _1));
	}

protected:
	static Ptr<ParamInfo::Enum> s_channelParam;

	virtual int getNumChannels() { return AudioIO::get().m_numInputChannels; }
};

////////////////////////////////////////////////////////////////////////////////

}; // end namespace Builtin
