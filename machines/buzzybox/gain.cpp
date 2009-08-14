#include "stdafx.h"
#include "buzzybox.h"

MachineInfo* Gain::getInfo()
{
	static MachineInfo info;
	static bool initialised = false;
	if (!initialised)
	{
		info.defaultName = "Gain";
		info.typeHint = MachineTypeHint::effect;

		static PinInfo inpin = { "Input", SignalType::stereoAudio };
		static const PinInfo* inpins[] = { &inpin, NULL };
		info.inPins = inpins;

		static PinInfo outpin = { "Output", SignalType::stereoAudio };
		static const PinInfo* outpins[] = { &outpin, NULL };
		info.outPins = outpins;

		static RealParamInfo paraGain;
		paraGain.p.tag = ptGain;
		paraGain.p.name = "Gain";
		paraGain.min = 0; paraGain.max = 2; paraGain.def = 1;
		
		static RealParamInfo paraPan;
		paraPan.p.tag = ptPan;
		paraPan.p.name = "Pan";
		paraPan.min = 0; paraPan.max = 2; paraPan.def = 1;

		static const ParamInfo* params[] = { &paraGain.p, &paraPan.p, NULL };
		info.params.params = params;
		
		initialised = true;
	}

	return &info;
}

void Gain::changeParam(ParamTag tag, double value)
{
	switch (tag)
	{
	case ptGain:
		m_gain = static_cast<float>(value);
		break;
	case ptPan:
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
