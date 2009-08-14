// sinetest.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

class SineTest : public Mi
{
public:
	enum { ptFreq, ptVol, };

	SineTest(HostMachine* hm) : Mi(hm), m_phase(0), m_amp(0.2)
	{
		setFreq(440.0 / 44100.0);
	}

	void setFreq(double freq)
	{
		m_phasestep = 2.0 * M_PI * freq;
	}

	void changeParam(ParamTag tag, double value)
	{
		switch (tag)
		{
		case ptFreq:
			setFreq(value);
			break;

		case ptVol:
			m_amp = value;
			break;
		}
	}

	void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe)
	{
		for (int i=firstframe; i<lastframe; i++)
		{
			outpins[0].f[i*2] = outpins[0].f[i*2+1]
				= (float)(sin(m_phase) * m_amp);
			m_phase += m_phasestep;
			if (m_phase > 2.0 * M_PI) m_phase -= 2.0 * M_PI;
		}
	}

	MachineInfo* getInfo()
	{
		static MachineInfo info;

		static bool initialised = false;
		if (!initialised)
		{
			info.defaultName = "sin";
			info.typeHint = MachineTypeHint::generator;

			using namespace TimeUnit;
			static TimeParamInfo paraFreq;
			paraFreq.p.tag = ptFreq;
			paraFreq.p.name = "Frequency";
			paraFreq.min.set(20, hertz); paraFreq.max.set(2000, hertz); paraFreq.def.set(440, hertz);
			paraFreq.internalUnit = fracfreq;
			paraFreq.displayUnits = hertz | notenum;
			paraFreq.defaultDisplayUnit = hertz;

			static RealParamInfo paraVol;
			paraVol.p.tag = ptVol;
			paraVol.p.name = "Volume";
			paraVol.min = 0; paraVol.max = 1; paraVol.def = 0.5;

			static const ParamInfo* params[] = { &paraFreq.p, &paraVol.p, NULL };
			info.params.params = params;

			static PinInfo outPin = { "Output", SignalType::stereoAudio };
			static const PinInfo* outPins[] = { &outPin, NULL };
			info.outPins = outPins;

			initialised = true;
		}

		return &info;
	}

protected:
	double m_phase, m_phasestep, m_amp;
};

void CosecantPlugin::enumerateFactories(MiFactoryList* list)
{
	g_host->registerMiFactory(list, "btdsys/test/sinedll", "BTDSys Bathroom Sink", NULL, 0);
}

Mi* CosecantPlugin::createMachine(const void*, unsigned int, HostMachine* hm)
{
	return new SineTest(hm);
}
