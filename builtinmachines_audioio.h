#pragma once

#include "builtinmachines.h"
#include "audioio.h"

namespace Builtin
{

///////////////////////////////////////////////////////////////////////

class AudioInOut : public QObject, public Mi
{
	Q_OBJECT

public:
	virtual void changeParam(ParamTag tag, ParamValue value)
	{
		qDebug() << "changeParam" << tag << value;
		if (tag == 'chan')
		{
			int c = static_cast<int>(value) - 1;
			m_leftChannel  = c*2;
			m_rightChannel = c*2+1;
			qDebug() << "channels" << m_leftChannel << m_rightChannel;
		}
	}

protected:
	int m_leftChannel, m_rightChannel;

	AudioInOut(Machine* mac, Callbacks* cb) : Mi(mac, cb), m_leftChannel(-1), m_rightChannel(-1)
	{
	}

	virtual int getNumChannels() = 0;

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
		Ptr<Parameter::Enum> param = dynamic_cast<Parameter::Enum*>(m_mac->m_paramMap.get('chan').c_ptr());
		param->setItems(getChannelParamItems(nChans, getName));
	}
};

////////////////////////////////////////////////////////////////////////////////

class AudioOut : public AudioInOut
{
	Q_OBJECT

public:
	AudioOut(Machine* mac, Callbacks* cb) : AudioInOut(mac, cb) {}

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

	static bool getInfo(InfoImpl::MachineInfo* info, InfoImpl::InfoCallbacks* cb)
	{
		info->setName("AOut")->setTypeHint(MachineTypeHint::master)
			->addInPin(cb->createPin()->setName("To soundcard")->setType(SignalType::stereoAudio));
		info->getParams()->addParam(
			cb->createEnumParam('chan')->setName("Channel")
			->setItems(getChannelParamItems(AudioIO::get().m_numOutputChannels,
				boost::bind(&AudioIO::getOutputChannelName, &AudioIO::get(), _1)
			))->setDefault(1)
		);
		return true;
	}

	void populateChannelParam()
	{
		AudioInOut::populateChannelParam(AudioIO::get().m_numOutputChannels,
			boost::bind(&AudioIO::getOutputChannelName, &AudioIO::get(), _1));
	}

protected:
	virtual int getNumChannels() { return AudioIO::get().m_numOutputChannels; }
};

////////////////////////////////////////////////////////////////////////////////

class AudioIn : public AudioInOut
{
	Q_OBJECT

public:
	AudioIn(Machine* mac, Callbacks* cb) : AudioInOut(mac, cb) {}

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

	static bool getInfo(InfoImpl::MachineInfo* info, InfoImpl::InfoCallbacks* cb)
	{
		info->setName("AIn")->setTypeHint(MachineTypeHint::master)
			->addOutPin(cb->createPin()->setName("From soundcard")->setType(SignalType::stereoAudio));
		info->getParams()->addParam(
			cb->createEnumParam('chan')->setName("Channel")->addItem("None") );
		return true;
	}

	void populateChannelParam()
	{
		AudioInOut::populateChannelParam(AudioIO::get().m_numInputChannels,
			boost::bind(&AudioIO::getInputChannelName, &AudioIO::get(), _1));
	}

protected:
	virtual int getNumChannels() { return AudioIO::get().m_numInputChannels; }
};

////////////////////////////////////////////////////////////////////////////////

}; // end namespace Builtin
