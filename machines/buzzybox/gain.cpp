#include "stdafx.h"
#include "buzzybox.h"

bool Gain::getInfo(MachineInfo* info, InfoCallbacks* cb)
{
	info->setName("Gain")->setTypeHint(MachineTypeHint::effect)
		->addInPin (cb->createPin()->setName("Input") ->setType(SignalType::stereoAudio))
		->addOutPin(cb->createPin()->setName("Output")->setType(SignalType::stereoAudio));

	info->getParams()
		->addParam(cb->createRealParam('gain')->setName("Gain")->setRange(0,2)->setDefault(1))
		->addParam(cb->createRealParam('pan ')->setName("Pan") ->setRange(0,2)->setDefault(1));

	return true;
}

Gain::Gain(HostMachine* mac, Callbacks* cb) : Mi(mac,cb), m_gain(1.0f), m_pan(1.0f)
{
}

void Gain::changeParam(ParamTag tag, ParamValue value)
{
	switch (tag)
	{
	case 'gain':
		m_gain = static_cast<float>(value);
		break;
	case 'pan ':
		m_pan = static_cast<float>(value);
		break;
	}
}

void Gain::work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe)
{
	for (int i=firstframe; i<lastframe; i++)
	{
		outpins[0].f[i*2  ] = inpins[0].f[i*2  ] * m_gain * (2.0f - m_pan);
		outpins[0].f[i*2+1] = inpins[0].f[i*2+1] * m_gain * m_pan;
	}
}
