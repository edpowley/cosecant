#pragma once

// sinetest.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

class TrackerTest;

class Pattern : public MiPattern
{
public:
	Pattern(TrackerTest* mi, double length);

	void play(SequenceTrack* track, double startpos);
	void stop(SequenceTrack* track);

	int getSubdiv() { return m_subdiv; }

	struct Cell
	{
		Cell() : note(-1), vel(-1) {}
		int note, vel;
	};

	typedef std::vector<Cell> Row;

	int getNumRows() { return m_data.size(); }
	const Row& getRow(int r) { return m_data[r]; }

protected:
	TrackerTest* m_mi;
	double m_length;
	int m_subdiv;
	std::vector<Row> m_data;
};

class PatternPlayer
{
public:
	PatternPlayer(TrackerTest* mi, Pattern* pat, double pos) : m_mi(mi), m_pat(pat), m_pos(pos) {}

	Pattern* getPattern() { return m_pat; }

	void work(PinBuffer* outpins, int firstframe, int lastframe);

protected:
	TrackerTest* m_mi;
	Pattern* m_pat;
	double m_pos;
};

typedef std::map<SequenceTrack*, PatternPlayer> PatternPlayerMap;

class TrackerTest : public Mi
{
public:
	TrackerTest(HostMachine* hm);
	MachineInfo* getInfo();
	void init();

	ScriptValue* callScriptFunction(int id, const ScriptValue** args, int numargs);

	void changeParam(ParamTag tag, double value);
	void work(const PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);

	MiPattern* createPattern(double length) { return new Pattern(this, length); }

	void playPattern(SequenceTrack* track, Pattern* p, double pos);
	void stopPattern(SequenceTrack* track, Pattern* p);

	double m_beatsPerFrame;

	int getNumTracks() { return m_nTracks; }

protected:
	PatternPlayerMap m_patternPlayers;
	int m_nTracks;
};
