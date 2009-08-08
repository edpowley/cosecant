// lfo.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

class Lfo : public Mi
{
public:
	MachineInfo* getInfo()
	{
		static MachineInfo info;

		static bool initialised = false;
		if (!initialised)
		{
			info.defaultName = "LFO";
			info.typeHint = MachineTypeHint::control;

			using namespace TimeUnit;
			static TimeParamInfo paraFreq;
			paraFreq.p.tag = COSECANT_TAG('sped');
			paraFreq.p.name = "Speed";
			paraFreq.min.set(2, samples); paraFreq.max.set(10, seconds); paraFreq.def.set(1, seconds);
			paraFreq.internalUnit = samples;
			paraFreq.displayUnits = seconds | samples | beats | hertz;
			paraFreq.defaultDisplayUnit = seconds;

			static IntParamInfo paraStep;
			paraStep.p.tag = COSECANT_TAG('step');
			paraStep.p.name = "Send step";
			paraStep.min = 1; paraStep.max = 256; paraStep.def = 16;

			static const ParamInfo* params[] = { &paraFreq.p, &paraStep.p, NULL };
			info.params.params = params;

			static PinInfo outPin = { "Output", SignalType::paramControl };
			static const PinInfo* outPins[] = { &outPin, NULL };
			info.outPins = outPins;

			initialised = true;
		}

		return &info;
	}

	Lfo(HostMachine* hm) : Mi(hm), m_phase(0), m_centre(0.5), m_amp(0.5),
		m_sendphase(0), m_sendphasemax(32)
	{
		setPeriod(1.0);
	}

	void setPeriod(double period)
	{
		m_phasestep = 2.0 * M_PI / period;
	}

	void changeParam(ParamTag tag, double value)
	{
		switch (tag)
		{
		case 'sped':
			setPeriod(value);
			break;
		case 'step':
			m_sendphasemax = (int)value;
			break;
		}
	}

	void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe)
	{
		for (int i=firstframe; i<lastframe; i++)
		{
			if (m_sendphase == 0)
			{
				g_host->addParamChangeEvent(&outpins[0], i, m_centre + sin(m_phase) * m_amp);
			}

			m_sendphase ++;
			if (m_sendphase >= m_sendphasemax) m_sendphase = 0;

			m_phase += m_phasestep;
			if (m_phase >= 2.0 * M_PI) m_phase -= 2.0 * M_PI;
		}
	}

protected:
	double m_phase, m_phasestep, m_centre, m_amp;
	int m_sendphase, m_sendphasemax;
};

void CosecantPlugin::enumerateFactories(MiFactoryList* list)
{
	g_host->registerMiFactory(list, "btdsys/lfo", "BTDSys LFO", NULL, 0);
}

Mi* CosecantPlugin::createMachine(const void*, unsigned int, HostMachine* hm)
{
	return new Lfo(hm);
}
