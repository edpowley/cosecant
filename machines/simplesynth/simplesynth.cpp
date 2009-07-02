// simplesynth.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

class SimpleSynth : public Mi
{
public:
	static bool getInfo(MachineInfo* info, const InfoCallbacks* cb);

	static const int c_polyphony = 16;

	SimpleSynth(HostMachine* mac, Callbacks* cb);

	class Note
	{
		friend class SimpleSynth;
	public:
		void init(SimpleSynth* mac);
		void noteOn(double note, double vel);
		void noteOff();

		void work(float* buffer, int numframes);

	protected:
		SimpleSynth* m_mac;
		double m_phase, m_phaseStep, m_amp;

		void setNote(double n);

		enum {attack, sustain, release, finished} m_envStage;
		double m_envAmp;
	};

	virtual void* noteOn(double note, double velocity);
	virtual void noteOff(void* note);

	virtual void changeParam(ParamTag tag, ParamValue value);
	virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);

protected:
	Note m_notes[c_polyphony];
	double m_envAttackStep, m_envReleaseStep;
};

////////////////////////////////////////////////////////////////////////////////////

bool SimpleSynth::getInfo(MachineInfo* info, const InfoCallbacks* cb)
{
	cb->setName(info, "Synth");
	cb->setTypeHint(info, MachineTypeHint::generator);
	cb->addFlags(info, MachineFlags::hasNoteTrigger | MachineFlags::createSequenceTrack);
	cb->addOutPin(info, "Output", SignalType::stereoAudio);

	ParamGroup* params = cb->createParamGroup("", 0);
	cb->setParams(info, params);
	cb->addTimeParam(params, "Attack", 'enva', TimeUnit::samples, 1, 44100.0 * 10.0, 44100.0 * 0.2,
		TimeUnit::seconds | TimeUnit::samples | TimeUnit::ticks, TimeUnit::seconds);
	cb->addTimeParam(params, "Release", 'envr', TimeUnit::samples, 1, 44100.0 * 10.0, 44100.0 * 1.0,
		TimeUnit::seconds | TimeUnit::samples | TimeUnit::ticks, TimeUnit::seconds);

	return true;
}

///////////////////////////////////////////////////////////////////////////////////

SimpleSynth::SimpleSynth(HostMachine* mac, Callbacks* cb)
: Mi(mac,cb)
{
	for (int n=0; n<c_polyphony; n++)
		m_notes[n].init(this);

	m_envAttackStep = 0.1;
	m_envReleaseStep = 0.1;
}

void SimpleSynth::Note::init(SimpleSynth* mac)
{
	m_mac = mac;
	m_envStage = finished;
}

//////////////////////////////////////////////////////////////////////////////////

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

void SimpleSynth::noteOff(void* note)
{
	if (note == NULL) // all notes off
		for (int i=0; i<c_polyphony; i++)
			m_notes[i].noteOff();
	else // one note off
		static_cast<Note*>(note)->noteOff();
}

void SimpleSynth::Note::noteOff()
{
	m_envStage = release;
}

//////////////////////////////////////////////////////////////////////////////////

void SimpleSynth::changeParam(ParamTag tag, ParamValue value)
{
	switch (tag)
	{
	case 'enva':
		m_envAttackStep = 1.0 / value;
		break;
	case 'envr':
		m_envReleaseStep = 1.0 / value;
		break;
	}
}

////////////////////////////////////////////////////////////////////////////

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

void SimpleSynth::work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe)
{
	// Clear output buffer
	for (int i=firstframe*2; i<lastframe*2; i++)
		outpins[0].f[i] = 0.0f;

	// Work each note in turn
	float* workbuf = outpins[0].f + firstframe*2;
	int workframes = lastframe - firstframe;
	for (int i=0; i<c_polyphony; i++)
	{
		if (m_notes[i].m_envStage != Note::finished)
			m_notes[i].work(workbuf, workframes);
	}
}

//////////////////////////////////////////////////////////////////////////

void CosecantAPI::populateMiFactories(std::map<std::string, MiFactory*>& factories)
{
	factories["btdsys/simplesynth"] = new MiFactory_T<SimpleSynth>;
}