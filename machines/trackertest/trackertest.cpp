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

TrackerTest::TrackerTest(Callbacks* cb) : Mi(cb)
{
}

void TrackerTest::init()
{
	m_cb->addScriptFunction("scriptTest", 27);
}

#include "script.inc"

const char* TrackerTest::getScript() { return g_script; }

Script::ValuePtr TrackerTest::callScriptFunction(int id, Script::Arguments* arg)
{
	char txt[1024];
	sprintf(txt,"You're in callScriptFunction! id=%i, arg count = %i", id, arg->count());
	m_cb->debugPrint(txt);
	return m_cb->scriptValueNull();
}

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
