#pragma once

// sinetest.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

class TrackerTest : public Mi
{
public:
	static bool getInfo(MachineInfo* info, InfoCallbacks* cb);

	TrackerTest(HostMachine* mac, Callbacks* cb);
	const char* getScript();

	void changeParam(ParamTag tag, ParamValue value);
	void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);
};
