#pragma once

// sinetest.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

class TrackerTest;

class Pattern : public MiPattern
{
public:
	Pattern(TrackerTest* mi, double length) : m_mi(mi), MiPattern(length) {}

protected:
	TrackerTest* m_mi;
};

class TrackerTest : public Mi
{
public:
	static bool getInfo(MachineInfo* info, InfoCallbacks* cb);

	TrackerTest(Callbacks* cb);
	void init();
	const char* getScript();

	Script::ValuePtr callScriptFunction(int id, Script::Arguments* arg);

	void changeParam(ParamTag tag, ParamValue value);
	void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);

	MiPattern* createPattern(double length) { return new Pattern(this, length); };
};
