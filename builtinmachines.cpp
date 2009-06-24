#include "stdafx.h"
#include "common.h"
#include "builtinmachines.h"
#include "machinfo.h"
#include "sequence.h"
//#include "notebookwindow.h"
#include "delayline.h"

#include "builtinmachines_audioio.h"

Ptr<ParamInfo::Enum> Builtin::AudioOut::s_channelParam;
Ptr<ParamInfo::Enum> Builtin::AudioIn::s_channelParam;

namespace Builtin
{

class AudioDelay : public Machine
{
public:
	static const int c_maxDelay = 44100;

	static Ptr<MachInfo> getInfo()
	{
		return (new MachInfo("builtin/adelay"))
			->setName("Delay")
			->setTypeHint(MachineTypeHint::effect)
			->addInPin (new PinInfo("In",  SignalType::stereoAudio))
			->addOutPin(new PinInfo("Out", SignalType::stereoAudio))
			->setParams((new ParamInfo::Group("", 0))
				->add(new ParamInfo::Time("Time", 'time', TimeUnit::samples, 1, c_maxDelay, 22050,
					TimeUnit::samples | TimeUnit::seconds | TimeUnit::ticks, TimeUnit::seconds))
			);
		;
	}

	AudioDelay()
	{
		m_line = new DelayLine::Audio(c_maxDelay, 2);
		setDelayTime(22050);
	}

	void setDelayTime(int t)
	{
		m_delayTime = t;
		m_line->setReadHead(m_line->getWriteHead() - t);
	}

	virtual void changeParam(ParamTag tag, ParamValue value)
	{
		switch (tag)
		{
		case 'time':
			setDelayTime(clamp((int)value, 1, c_maxDelay));
			break;
		}
	}

	virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe)
	{
		for (int i=firstframe; i<lastframe; i += m_delayTime)
		{
			int j = min(lastframe, i + m_delayTime);
			m_line->read(outpins[0].workbuffer, i, j);
			m_line->write(inpins[0].workbuffer, i, j);
		}
	}

protected:
	Ptr<DelayLine::Audio> m_line;
	int m_delayTime;
};

}; // end namespace Builtin

///////////////////////////////////////////////////////////////////////////

void initBuiltinMachineFactories()
{
	MachineFactory::add("builtin/aout",		new BuiltinMachineFactory<Builtin::AudioOut>);
	MachineFactory::add("builtin/ain",		new BuiltinMachineFactory<Builtin::AudioIn>);
	MachineFactory::add("builtin/adelay",	new BuiltinMachineFactory<Builtin::AudioDelay>);
	MachineFactory::add("builtin/dummy",	new BuiltinMachineFactory<DummyMachine>);
}
