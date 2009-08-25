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
	}

	void work(const WorkContext* ctx)
	{
		ESIter enditer = ESIter::upperBound(ctx->ev, ctx->firstframe);
		for (ESIter iter = ESIter::lowerBound(ctx->ev, ctx->firstframe); iter != enditer; ++iter)
		{
			StreamEvent ev = iter.value();
			switch (ev.type)
			{
			case StreamEventType::paramChange:
				switch (ev.paramChange.tag)
				{
				case ptFreq:
					setFreq(ev.paramChange.value);
					break;

				case ptVol:
					m_amp = ev.paramChange.value;
					break;
				}
			}
		}

		for (int i=ctx->firstframe; i<ctx->lastframe; i++)
		{
			ctx->out[0].f[i*2] = ctx->out[0].f[i*2+1]
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

			static ParamInfo* params[] = { &paraFreq.p, &paraVol.p, NULL };
			info.params.params = params;

			static PinInfo outPin("Output", SignalType::stereoAudio);
			static PinInfo* outPins[] = { &outPin, NULL };
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
