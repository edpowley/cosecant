#pragma once

#include "stdafx.h"
#include "builtinmachines.h"
#include "audioio.h"
#include "parameter.h"

namespace Builtin
{

///////////////////////////////////////////////////////////////////////

class AudioInOut : public BuiltinMachine
{
	Q_OBJECT

public:
	enum
	{
		ptChannel,
	};

protected:
	int m_leftChannel, m_rightChannel;

	AudioInOut() : m_leftChannel(-1), m_rightChannel(-1)
	{
	}

	void processEvents(const WorkContext* ctx)
	{
		const WorkBuffer::Events* evbuf = static_cast<const WorkBuffer::Events*>(ctx->ev->hostbuf);
		foreach(const StreamEvent& ev, evbuf->m_data)
		{
			if (ev.type == StreamEventType::paramChange && ev.paramChange.tag == ptChannel)
			{
				int c = static_cast<int>(ev.paramChange.value) - 1;
				m_leftChannel  = c*2;
				m_rightChannel = c*2+1;
			}
		}
	}

	virtual int getNumChannels() = 0;

	static const EnumParamInfo& getChannelParamInfo()
	{
		static EnumParamInfo info;
		static bool initialised = false;
		if (!initialised)
		{
			info.p.tag = ptChannel;
			info.p.name = "Channel";
			static const char* items[] = {"None", "Channel 1/2", NULL};
			info.items = items;
			info.def = 1;
			initialised = true;
		}
		return info;
	}

    static QStringList getChannelParamItems(int nChans, std::function<QString(int)> getName)
	{
		QStringList items("None");
		for (int i=0; i<nChans/2; i++)
		{
			items << QString("%1, %2").arg(getName(i*2), getName(i*2+1));
		}
		return items;
	}

    void populateChannelParam(int nChans, std::function<QString(int)> getName)
	{
		Ptr<Parameter::Enum> param = dynamic_cast<Parameter::Enum*>(
			m_paramMap.value(ptChannel).c_ptr() );
		param->setItems(getChannelParamItems(nChans, getName));
	}
};

////////////////////////////////////////////////////////////////////////////////

class AudioOut : public AudioInOut
{
	Q_OBJECT

public:
	AudioOut() : AudioInOut() {}

	virtual void work(const WorkContext* ctx)
	{
		processEvents(ctx);

		int nChans = AudioIO::get().m_numOutputChannels;
		if (m_leftChannel  < 0 || m_leftChannel  >= nChans) return;
		if (m_rightChannel < 0 || m_rightChannel >= nChans) return;

		CSC_LOCK_MUTEX(&AudioIO::get().m_outmutex);

		for (int i=ctx->firstframe; i<ctx->lastframe; i++)
		{
			AudioIO::get().m_outbuf[i * nChans + m_leftChannel ] += ctx->in[0].f[i*2];
			AudioIO::get().m_outbuf[i * nChans + m_rightChannel] += ctx->in[0].f[i*2+1];
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

			static PinInfo pin("To soundcard", SignalType::stereoAudio);
			static PinInfo* pins[] = { &pin, NULL };
			info.inPins = pins;

			initialised = true;
		}

		m_info = &info;
	}

	void populateChannelParam()
	{
		AudioInOut::populateChannelParam(AudioIO::get().m_numOutputChannels,
            std::bind(&AudioIO::getOutputChannelName, &AudioIO::get(), std::placeholders::_1));
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

	virtual void work(const WorkContext* ctx)
	{
		processEvents(ctx);

		int nChans = AudioIO::get().m_numInputChannels;
		if (m_leftChannel  < 0 || m_leftChannel  >= nChans) return;
		if (m_rightChannel < 0 || m_rightChannel >= nChans) return;

		for (int i=ctx->firstframe; i<ctx->lastframe; i++)
		{
			ctx->out[0].f[i*2  ] = AudioIO::get().m_inbuf[i * nChans + m_leftChannel];
			ctx->out[0].f[i*2+1] = AudioIO::get().m_inbuf[i * nChans + m_rightChannel];
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

			static PinInfo pin("From soundcard", SignalType::stereoAudio);
			static PinInfo* pins[] = { &pin, NULL };
			info.outPins = pins;

			initialised = true;
		}

		m_info = &info;
	}

	void populateChannelParam()
	{
		AudioInOut::populateChannelParam(AudioIO::get().m_numInputChannels,
            std::bind(&AudioIO::getInputChannelName, &AudioIO::get(), std::placeholders::_1));
	}

protected:
	virtual void initImpl() { populateChannelParam(); }
	virtual int getNumChannels() { return AudioIO::get().m_numInputChannels; }
};

////////////////////////////////////////////////////////////////////////////////

}; // end namespace Builtin
