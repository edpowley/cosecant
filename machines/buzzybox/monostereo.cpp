#include "stdafx.h"
#include "buzzybox.h"

bool StereoToMonoMono::getInfo(MachineInfo* info, const InfoCallbacks* cb)
{
	cb->setName(info, "Stereo to 2\xc3\x97Mono");
	cb->setTypeHint(info, MachineTypeHint::effect);

	cb->addInPin (info, "Input",    SignalType::stereoAudio);
	cb->addOutPin(info, "Output L", SignalType::monoAudio);
	cb->addOutPin(info, "Output R", SignalType::monoAudio);

	return true;
}

bool MonoMonoToStereo::getInfo(MachineInfo* info, const InfoCallbacks* cb)
{
	cb->setName(info, "2\xc3\x97Mono to Stereo");
	cb->setTypeHint(info, MachineTypeHint::effect);

	cb->addInPin (info, "Input L", SignalType::monoAudio);
	cb->addInPin (info, "Input L", SignalType::monoAudio);
	cb->addOutPin(info, "Output",  SignalType::stereoAudio);

	return true;
}

void StereoToMonoMono::work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe)
{
	for (int i=firstframe; i<lastframe; i++)
	{
		outpins[0].f[i] = inpins[0].f[i*2];
		outpins[1].f[i] = inpins[0].f[i*2+1];
	}
}

void MonoMonoToStereo::work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe)
{
	for (int i=firstframe; i<lastframe; i++)
	{
		outpins[0].f[i*2]   = inpins[0].f[i];
		outpins[0].f[i*2+1] = inpins[1].f[i];
	}
}
