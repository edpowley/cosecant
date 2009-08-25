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

		static PinInfo inpin("Input", SignalType::stereoAudio);
		static const PinInfo* inpins[] = { &inpin, NULL };
		info.inPins = inpins;

		static PinInfo outpin("Output", SignalType::stereoAudio);
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

void Gain::work(const WorkContext* ctx)
{
	ESIter enditer = ESIter::upperBound(ctx->ev, ctx->firstframe);
	for (Helper::ESIter iter = ESIter::lowerBound(ctx->ev, ctx->firstframe); iter != enditer; ++iter)
	{
		StreamEvent ev = iter.value();
		switch (ev.type)
		{
		case StreamEventType::paramChange:
			switch (ev.paramChange.tag)
			{
			case ptGain:
				m_gain = static_cast<float>(ev.paramChange.value);
				break;
			case ptPan:
				m_pan = static_cast<float>(ev.paramChange.value);
				break;
			}
			break;
		}
	}

	for (int i=ctx->firstframe; i<ctx->lastframe; i++)
	{
		ctx->out[0].f[i*2  ] = ctx->in[0].f[i*2  ] * m_gain * (2.0f - m_pan);
		ctx->out[0].f[i*2+1] = ctx->in[0].f[i*2+1] * m_gain * m_pan;
	}
}
