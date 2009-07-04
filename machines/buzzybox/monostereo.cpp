#include "stdafx.h"
#include "buzzybox.h"

bool StereoToMonoMono::getInfo(MachineInfo* info, InfoCallbacks* cb)
{
	info->setName("Stereo to 2\xc3\x97Mono")->setTypeHint(MachineTypeHint::effect)
		->addInPin (cb->createPin()->setName("Input")	->setType(SignalType::stereoAudio))
		->addOutPin(cb->createPin()->setName("Output L")->setType(SignalType::monoAudio))
		->addOutPin(cb->createPin()->setName("Output R")->setType(SignalType::monoAudio));

	return true;
}

bool MonoMonoToStereo::getInfo(MachineInfo* info, InfoCallbacks* cb)
{
	info->setName("2\xc3\x97Mono to Stereo")->setTypeHint(MachineTypeHint::effect)
		->addInPin (cb->createPin()->setName("Input L")	->setType(SignalType::monoAudio))
		->addInPin (cb->createPin()->setName("Input R")	->setType(SignalType::monoAudio))
		->addOutPin(cb->createPin()->setName("Output")	->setType(SignalType::stereoAudio));

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
