#include "stdafx.h"
#include "buzzybox.h"

MachineInfo* StereoToMonoMono::getInfo()
{
	static MachineInfo info;
	static bool initialised = false;
	if (!initialised)
	{
		info.defaultName = "Stereo to 2 &times; Mono";
		info.typeHint = MachineTypeHint::effect;

		static PinInfo inpin("Input", SignalType::stereoAudio);
		static const PinInfo* inpins[] = { &inpin, NULL };
		info.inPins = inpins;

		static PinInfo outpinL("Output L", SignalType::monoAudio);
		static PinInfo outpinR("Output R", SignalType::monoAudio);
		static const PinInfo* outpins[] = { &outpinL, &outpinR, NULL };
		info.outPins = outpins;

		initialised = true;
	}

	return &info;
}

void StereoToMonoMono::work(const WorkContext* ctx)
{
	for (int i=ctx->firstframe; i<ctx->lastframe; i++)
	{
		ctx->out[0].f[i] = ctx->in[0].f[i*2];
		ctx->out[1].f[i] = ctx->in[0].f[i*2+1];
	}
}

///////////////////////////////////////////////////////////////////////////////

MachineInfo* MonoMonoToStereo::getInfo()
{
	static MachineInfo info;
	static bool initialised = false;
	if (!initialised)
	{
		info.defaultName = "2 &times; Mono to Stereo";
		info.typeHint = MachineTypeHint::effect;

		static PinInfo inpinL("Input L", SignalType::monoAudio);
		static PinInfo inpinR("Input R", SignalType::monoAudio);
		static const PinInfo* inpins[] = { &inpinL, &inpinR, NULL };
		info.inPins = inpins;

		static PinInfo outpin("Output", SignalType::stereoAudio);
		static const PinInfo* outpins[] = { &outpin, NULL };
		info.outPins = outpins;

		initialised = true;
	}

	return &info;
}

void MonoMonoToStereo::work(const WorkContext* ctx)
{
	for (int i=ctx->firstframe; i<ctx->lastframe; i++)
	{
		ctx->out[0].f[i*2]   = ctx->in[0].f[i];
		ctx->out[0].f[i*2+1] = ctx->in[1].f[i];
	}
}
