// lfo.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

class Lfo : public Mi
{
public:
	enum { ptSpeed, ptStep, ptMin, ptMax, };

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
			paraFreq.p.tag = ptSpeed;
			paraFreq.p.name = "Speed";
			paraFreq.min.set(2, samples); paraFreq.max.set(10, seconds); paraFreq.def.set(1, seconds);
			paraFreq.internalUnit = samples;
			paraFreq.displayUnits = seconds | samples | beats | hertz;
			paraFreq.defaultDisplayUnit = seconds;

			static IntParamInfo paraStep;
			paraStep.p.tag = ptStep;
			paraStep.p.name = "Send step";
			paraStep.min = 1; paraStep.max = 256; paraStep.def = 16;

			static RealParamInfo paraMin, paraMax;
			paraMin.p.tag = ptMin;
			paraMin.p.name = "Minimum";
			paraMin.p.flags = ParamFlags::noMinMax;
			paraMin.def = 0;
			paraMax.p.tag = ptMax;
			paraMax.p.name = "Maximum";
			paraMax.p.flags = ParamFlags::noMinMax;
			paraMax.def = 1;

			static const ParamInfo* params[] = { &paraFreq.p, &paraStep.p, &paraMin.p, &paraMax.p, NULL };
			info.params.params = params;

			static PinInfo outPin("Output", SignalType::paramControl);
			static const PinInfo* outPins[] = { &outPin, NULL };
			info.outPins = outPins;

			initialised = true;
		}

		return &info;
	}

	Lfo(HostMachine* hm) : Mi(hm), m_phase(0), m_min(0.0), m_max(1.0),
		m_sendphase(0), m_sendphasemax(32)
	{
		setPeriod(1.0);
	}

	void setPeriod(double period)
	{
		m_phasestep = 2.0 * M_PI / period;
	}

	void work(const WorkContext* ctx)
	{
		ESIter enditer = ESIter::upperBound(ctx->ev, ctx->firstframe);
		for (Helper::ESIter iter = ESIter::lowerBound(ctx->ev, ctx->firstframe); iter != enditer; ++iter)
		{
			StreamEvent ev = iter.value();
			switch (ev.type)
			{
			case StreamEventType::paramChange:
				switch (ev.paramChange.tag)
				{
				case ptSpeed:
					setPeriod(ev.paramChange.value);
					break;
				case ptStep:
					m_sendphasemax = (int)ev.paramChange.value;
					break;
				case ptMin:
					m_min = ev.paramChange.value;
					break;
				case ptMax:
					m_max = ev.paramChange.value;
					break;
				}
				break;
			}
		}

		for (int i=ctx->firstframe; i<ctx->lastframe; i++)
		{
			if (m_sendphase == 0)
			{
				double v = 0.5 * sin(m_phase) + 0.5;
				v = m_min + v * (m_max - m_min);
				g_host->addParamChangeEvent(&ctx->out[0], i, v);
			}

			m_sendphase ++;
			if (m_sendphase >= m_sendphasemax) m_sendphase = 0;

			m_phase += m_phasestep;
			if (m_phase >= 2.0 * M_PI) m_phase -= 2.0 * M_PI;
		}
	}

protected:
	double m_phase, m_phasestep, m_min, m_max;
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
