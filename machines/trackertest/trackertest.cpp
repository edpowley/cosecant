// trackertest.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "trackertest.h"

bool TrackerTest::getInfo(MachineInfo* info, InfoCallbacks* cb)
{
	info->setName("Tracker")->setTypeHint(MachineTypeHint::control)
		->addFlags(	MachineFlags::createSequenceTrack
				|	MachineFlags::hasCustomPatterns
				|	MachineFlags::hasScript )
		->addOutPin(cb->createPin()->setName("Note")->setType(SignalType::noteTrigger));

	return true;
}

TrackerTest::TrackerTest(HostMachine* mac, Callbacks* cb) : Mi(mac,cb)
{
}

#include "script.inc"

const char* TrackerTest::getScript() { return g_script; }

void TrackerTest::changeParam(ParamTag tag, ParamValue value)
{
}

void TrackerTest::work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe)
{
}

void CosecantAPI::populateMiFactories(std::map<std::string, MiFactory*>& factories)
{
	factories["btdsys/test/tracker"] = new MiFactory_T<TrackerTest>;
}
