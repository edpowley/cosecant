// trackertest.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "trackertest.h"

using namespace CosecantAPI;

#include "script.inc"

MachineInfo* TrackerTest::getInfo()
{
	static MachineInfo info;

	static bool initialised = false;
	if (!initialised)
	{
		info.defaultName = "Tracker";
		info.typeHint = MachineTypeHint::control;
		info.flags = MachineFlags::createSequenceTrack 
					| MachineFlags::hasCustomPatterns;
		info.script = g_script;

		static PinInfo pinNoteOut = { "Note", SignalType::noteTrigger };
		static const PinInfo* outPins[] = { &pinNoteOut, NULL };
		info.outPins = outPins;

		initialised = true;
	}

	return &info;
}

TrackerTest::TrackerTest(HostMachine* hm) : Mi(hm)
{
}

void TrackerTest::init()
{
	g_host->registerScriptFunction(m_hm, "scriptTest", 27);
}

Variant TrackerTest::callScriptFunction(int id, const ScriptValue** args, int numargs)
{
	DebugPrint() << "You're in callScriptFunction!" << id << numargs;
	return Variant();
}

void TrackerTest::changeParam(ParamTag tag, double value)
{
}

void TrackerTest::work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe)
{
}

/////////////////////////////////////////////////////////////////////////////

void Pattern::play(SequenceTrack* track, double startpos)
{
}

void Pattern::stop(SequenceTrack* track)
{
}

///////////////////////////////////////////////////////////////////////////

void CosecantPlugin::enumerateFactories(MiFactoryList* list)
{
	g_host->registerMiFactory(list, "btdsys/test/tracker", "BTDSys Crappy Tracker Example", NULL, 0);
}

Mi* CosecantPlugin::createMachine(const void*, unsigned int, HostMachine* hm)
{
	return new TrackerTest(hm);
}
