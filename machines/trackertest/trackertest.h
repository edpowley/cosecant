#pragma once

// sinetest.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

class TrackerTest;

class Pattern : public MiPattern
{
public:
	Pattern(TrackerTest* mi) : m_mi(mi) {}

protected:
	TrackerTest* m_mi;
};

class TrackerTest : public Mi
{
public:
	static bool getInfo(MachineInfo* info, InfoCallbacks* cb);

	TrackerTest(Callbacks* cb);
	const char* getScript();

	void changeParam(ParamTag tag, ParamValue value);
	void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);

	MiPattern* createPattern() { return new Pattern(this); }
};
