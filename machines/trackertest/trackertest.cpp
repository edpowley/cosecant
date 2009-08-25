// trackertest.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "trackertest.h"

using namespace CosecantAPI;

enum ScriptFunctions
{
	sf_getNumRows,
	sf_getSubdiv,
	sf_getNumTracks,
	sf_getCell,
	sf_setCell,
};

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

		static PinInfo pinNoteOut("Note", SignalType::noteTrigger);
		static const PinInfo* outPins[] = { &pinNoteOut, NULL };
		info.outPins = outPins;

		initialised = true;
	}

	return &info;
}

TrackerTest::TrackerTest(HostMachine* hm) : Mi(hm), m_nTracks(4)
{
}

void TrackerTest::init()
{
	g_host->registerScriptFunction(m_hm, "getNumRows",		sf_getNumRows);
	g_host->registerScriptFunction(m_hm, "getSubdiv",		sf_getSubdiv);
	g_host->registerScriptFunction(m_hm, "getNumTracks",	sf_getNumTracks);
	g_host->registerScriptFunction(m_hm, "getCell",			sf_getCell);
	g_host->registerScriptFunction(m_hm, "setCell",			sf_setCell);
}

ScriptValue* TrackerTest::callScriptFunction(int id, const ScriptValue** args, int numargs)
{
	switch (id)
	{
	case sf_getNumRows:
		{
			if (numargs != 1) return NULL;
			Pattern* pat = dynamic_cast<Pattern*>(g_host->ScriptValue_toMiPattern(args[0]));
			if (!pat) return NULL;

			return g_host->ScriptValue_createInt(pat->getNumRows());
		}
		break;

	case sf_getSubdiv:
		{
			if (numargs != 1) return NULL;
			Pattern* pat = dynamic_cast<Pattern*>(g_host->ScriptValue_toMiPattern(args[0]));
			if (!pat) return NULL;

			return g_host->ScriptValue_createInt(pat->getSubdiv());
		}
		break;

	case sf_getNumTracks:
		return g_host->ScriptValue_createInt(m_nTracks);

	case sf_getCell:
		{
			if (numargs != 3) return NULL;
			Pattern* pat = dynamic_cast<Pattern*>(g_host->ScriptValue_toMiPattern(args[0]));
			if (!pat) return NULL;
			int row = g_host->ScriptValue_toInt(args[1]);
			int track = g_host->ScriptValue_toInt(args[2]);
			
			Pattern::Cell cell;
			if (row >= 0 && row < pat->getNumRows())
			{
				if (track >= 0 && track < (int)pat->getRow(row).size())
					cell = pat->getRow(row)[track];
			}

			ScriptValue* ret = g_host->ScriptValue_createArray(2);
			g_host->ScriptValue_setArrayElement(ret, 0, g_host->ScriptValue_createInt(cell.note), ctrue);
			g_host->ScriptValue_setArrayElement(ret, 1, g_host->ScriptValue_createInt(cell.vel) , ctrue);

			return ret;
		}
		break;

	case sf_setCell:
		break;
	}

	return NULL;
}

void TrackerTest::work(const WorkContext* ctx)
{
	const TimeInfo* timeinfo = g_host->getTimeInfo(m_hm);
	m_beatsPerFrame = timeinfo->beatsPerSecond / timeinfo->samplesPerSecond;
	for (PatternPlayerMap::iterator iter = m_patternPlayers.begin(); iter != m_patternPlayers.end(); ++iter)
	{
		iter->second.work(ctx->out, ctx->firstframe, ctx->lastframe);
	}
}

void PatternPlayer::work(PinBuffer* outpins, int firstframe, int lastframe)
{
	double subdivsPerFrame = m_mi->m_beatsPerFrame * (double)m_pat->getSubdiv();
	double nextpos = m_pos + subdivsPerFrame * (lastframe - firstframe);

	for (int r = (int)ceil(m_pos); r < (int)ceil(nextpos); r++)
	{
		if (r < 0 || r >= m_pat->getNumRows()) continue;

		int frame = firstframe + (int)floor((r - m_pos) / subdivsPerFrame);
		const Pattern::Row& row = m_pat->getRow(r);

		for (Pattern::Row::const_iterator iter = row.begin(); iter != row.end(); ++iter)
		{
		}
	}

	m_pos = nextpos;
}

/////////////////////////////////////////////////////////////////////////////

Pattern::Pattern(TrackerTest* mi, double length) : m_mi(mi), m_length(length), m_subdiv(4)
{
	Row row(m_mi->getNumTracks());
	m_data.resize((int)ceil(m_length * m_subdiv), row);
}

void Pattern::play(SequenceTrack* track, double startpos)
{
	m_mi->playPattern(track, this, startpos * m_subdiv);
}

void Pattern::stop(SequenceTrack* track)
{
	m_mi->stopPattern(track, this);
}

void TrackerTest::playPattern(SequenceTrack* track, Pattern* p, double startpos)
{
	m_patternPlayers.insert(std::make_pair( track, PatternPlayer(this, p, startpos) ));
}

void TrackerTest::stopPattern(SequenceTrack* track, Pattern* p)
{
	PatternPlayerMap::iterator iter = m_patternPlayers.find(track);
	if (iter != m_patternPlayers.end() && iter->second.getPattern() == p)
	{
		m_patternPlayers.erase(iter);		
	}
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
