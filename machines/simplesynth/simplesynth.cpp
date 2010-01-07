// simplesynth.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

class SimpleSynth : public Mi
{
public:
	enum { ptEnvA, ptEnvR, };

	MachineInfo* getInfo();

	SimpleSynth(HostMachine* hm);

	class Note
	{
		friend class SimpleSynth;
	public:
		Note(SimpleSynth* mac);

		void noteOn(double note, double vel);
		void noteOff();

		void work(float* buffer, int numframes);

	protected:
		SimpleSynth* m_mac;
		double m_phase, m_phaseStep, m_amp;

		void setNote(double n);

		enum {unstarted, attack, sustain, release, finished} m_envStage;
		double m_envAmp;
	};

	virtual void work(const WorkContext* ctx);

protected:
	std::map<void*, Note*> m_notes;
	double m_envAttackStep, m_envReleaseStep;
};

////////////////////////////////////////////////////////////////////////////////////

MachineInfo* SimpleSynth::getInfo()
{
	static MachineInfo info;

	static bool initialised = false;
	if (!initialised)
	{
		info.defaultName = "Synth";
		info.typeHint = MachineTypeHint::generator;

		using namespace TimeUnit;
		static TimeParamInfo paraAttack, paraRelease;

		paraAttack.p.tag = ptEnvA;
		paraAttack.p.name = "Attack";
		paraAttack.min.set(1, samples); paraAttack.max.set(10, seconds); paraAttack.def.set(0.2, seconds);
		paraAttack.internalUnit = samples;
		paraAttack.displayUnits = seconds | samples | beats;
		paraAttack.defaultDisplayUnit = seconds;

		paraRelease.p.tag = ptEnvR;
		paraRelease.p.name = "Release";
		paraRelease.min.set(1, samples); paraRelease.max.set(10, seconds); paraRelease.def.set(1, seconds);
		paraRelease.internalUnit = samples;
		paraRelease.displayUnits = seconds | samples | beats;
		paraRelease.defaultDisplayUnit = seconds;

		static const ParamInfo* params[] = { &paraAttack.p, &paraRelease.p, NULL };
		info.params.params = params;

		static PinInfo noteInPin("Note", SignalType::noteTrigger, PinFlags::breakOnEvent);
		static const PinInfo* inPins[] = { &noteInPin, NULL };
		info.inPins = inPins;

		static PinInfo outPin("Output", SignalType::stereoAudio);
		static const PinInfo* outPins[] = { &outPin, NULL };
		info.outPins = outPins;

		info.flags = MachineFlags::createSequenceTrack;

		initialised = true;
	}

	return &info;
}

///////////////////////////////////////////////////////////////////////////////////

SimpleSynth::SimpleSynth(HostMachine* hm)
: Mi(hm)
{
	m_envAttackStep = 0.1;
	m_envReleaseStep = 0.1;
}

SimpleSynth::Note::Note(SimpleSynth* mac) : m_mac(mac), m_envStage(unstarted)
{
}

//////////////////////////////////////////////////////////////////////////////////

/*
void* SimpleSynth::noteOn(double note, double velocity)
{
	// Find an inactive note, or a releasing note with minimal amplitude
	double minAmp = 1.0e10;
	Note* minAmpNote = NULL;

	for (int i=0; i<c_polyphony; i++)
	{
		if (m_notes[i].m_envStage == Note::finished)
		{
			m_notes[i].noteOn(note, velocity);
			return &m_notes[i];
		}
		else if (m_notes[i].m_envStage == Note::release && m_notes[i].m_envAmp < minAmp)
		{
			minAmpNote = &m_notes[i];
			minAmp = minAmpNote->m_envAmp;
		}
	}

	if (minAmpNote)
	{
		minAmpNote->noteOn(note, velocity);
		return static_cast<void*>(minAmpNote);
	}

	// The next step might be to look for the oldest note, or dynamically allocate a new note,
	// but this is only supposed to be a simple example so let's just use note 0.
	m_notes[0].noteOn(note, velocity);
	return static_cast<void*>(&m_notes[0]);
}
*/

void SimpleSynth::Note::noteOn(double note, double vel)
{
	setNote(note);
	m_amp = vel;
	m_envStage = attack;
	m_envAmp = 0.0;
	m_phase = 0.0;
}

void SimpleSynth::Note::setNote(double n)
{
	double freq = 440.0 * pow(2.0, (n - 69.0) / 12.0);
	m_phaseStep = 2.0 * M_PI * freq / 44100.0;
}

////////////////////////////////////////////////////////////////////////////////
/*
void SimpleSynth::noteOff(void* note)
{
	if (note == NULL) // all notes off
		for (int i=0; i<c_polyphony; i++)
			m_notes[i].noteOff();
	else // one note off
		static_cast<Note*>(note)->noteOff();
}
*/

void SimpleSynth::Note::noteOff()
{
	m_envStage = release;
}

//////////////////////////////////////////////////////////////////////////////////

void SimpleSynth::Note::work(float* buffer, int numframes)
{
	for (int i=0; i<numframes; i++)
	{
		float samp = (float)(sin(m_phase) * m_amp * m_envAmp);
		buffer[i*2] += samp; buffer[i*2+1] += samp;
		m_phase += m_phaseStep;
		if (m_phase > 2.0 * M_PI) m_phase -= 2.0 * M_PI;

		switch (m_envStage)
		{
		case attack:
			m_envAmp += m_mac->m_envAttackStep;
			if (m_envAmp >= 1.0)
			{
				m_envAmp = 1.0;
				m_envStage = sustain;
			}
			break;

		case release:
			m_envAmp -= m_mac->m_envReleaseStep;
			if (m_envAmp <= 0.0)
			{
				m_envAmp = 0.0;
				m_envStage = finished;
				return;
			}
			break;
		}
	}
}

void SimpleSynth::work(const WorkContext* ctx)
{
	using namespace CosecantAPI::Helper;

	// Process global events
	ESIter enditer = ESIter::upperBound(ctx->ev, ctx->firstframe);
	for (ESIter iter = ESIter::lowerBound(ctx->ev, ctx->firstframe); iter != enditer; ++iter)
	{
		StreamEvent ev = iter.value();
		switch (ev.type)
		{
		case StreamEventType::paramChange:
			switch (ev.paramChange.tag)
			{
			case ptEnvA:
				m_envAttackStep = 1.0 / ev.paramChange.value;
				break;
			case ptEnvR:
				m_envReleaseStep = 1.0 / ev.paramChange.value;
				break;
			}
			break;
		}
	}

	// Process note triggers
	EventStreamIterator upper = EventStreamIterator::upperBound(ctx->in+0, ctx->firstframe);
	for (EventStreamIterator iter = EventStreamIterator::lowerBound(ctx->in+0, ctx->firstframe);
		iter != upper; ++iter)
	{
		StreamEvent ev = iter.value();
		switch (ev.type)
		{
		case StreamEventType::noteOn:
			DebugPrint() << "note on" << ev.note.id << ev.note.note << ev.note.vel;
			{
				Note* note = new Note(this);
				note->noteOn(ev.note.note, ev.note.vel);
				m_notes.insert(std::make_pair(ev.note.id, note));
			}
			break;

		case StreamEventType::noteOff:
			DebugPrint() << "note off" << ev.note.id;
			{
				std::map<void*, Note*>::iterator noteiter = m_notes.find(ev.note.id);
				if (noteiter != m_notes.end())
					noteiter->second->noteOff();
				else
					DebugPrint() << "couldn't find note";
			}
			break;
		}
	}

	// Clear output buffer
	for (int i=ctx->firstframe*2; i<ctx->lastframe*2; i++)
		ctx->out[0].f[i] = 0.0f;

	// Work each note in turn
	float* workbuf = ctx->out[0].f + ctx->firstframe*2;
	int workframes = ctx->lastframe - ctx->firstframe;
	for (std::map<void*, Note*>::iterator iter = m_notes.begin(); iter != m_notes.end(); /* ++ inside loop */)
	{
		iter->second->work(workbuf, workframes);
		if (iter->second->m_envStage == Note::finished)
		{
			delete iter->second;
			iter = m_notes.erase(iter);
		}
		else
			++iter;
	}
}

//////////////////////////////////////////////////////////////////////////

void CosecantPlugin::enumerateFactories(MiFactoryList* list)
{
	g_host->registerMiFactory(list, "btdsys/simplesynth", "BTDSys Simple Synth", NULL, 0);
}

Mi* CosecantPlugin::createMachine(const void*, unsigned int, HostMachine* hm)
{
	return new SimpleSynth(hm);
}
