#pragma once

// sinetest.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

class TrackerTest;

class Pattern : public MiPattern
{
public:
	Pattern(TrackerTest* mi, double length) : m_mi(mi), m_length(length) {}

	void play(SequenceTrack* track, double startpos);
	void stop(SequenceTrack* track);

protected:
	TrackerTest* m_mi;
	double m_length;
};

class TrackerTest : public Mi
{
public:
	TrackerTest(HostMachine* hm);
	MachineInfo* getInfo();
	void init();

	Variant callScriptFunction(int id, const ScriptValue** args, int numargs);

	void changeParam(ParamTag tag, double value);
	void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);

	MiPattern* createPattern(double length) { return new Pattern(this, length); }
};
