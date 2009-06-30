#include "stdafx.h"
#include "buzzybox.h"

bool Gain::getInfo(MachineInfo* info, const InfoCallbacks* cb)
{
	cb->setName(info, "Gain");
	cb->setTypeHint(info, MachineTypeHint::effect);

	cb->addInPin (info, "Input",  SignalType::stereoAudio);
	cb->addOutPin(info, "Output", SignalType::stereoAudio);

	ParamGroup* params = cb->createParamGroup("", 0);
	cb->setParams(info, params);
	cb->addRealParam(params, "Gain", 'gain', 0.0, 2.0, 1.0, 0);
	cb->addRealParam(params, "Pan",  'pan ', 0.0, 2.0, 1.0, 0);

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
