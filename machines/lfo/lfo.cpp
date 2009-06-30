// lfo.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

class Lfo : public Mi
{
public:
	static bool getInfo(MachineInfo* info, const InfoCallbacks* cb)
	{
		cb->setName(info, "LFO");
		cb->setTypeHint(info, MachineTypeHint::control);

		cb->addOutPin(info, "Output", SignalType::paramControl);

		ParamGroup* params = cb->createParamGroup("", 0);
		cb->setParams(info, params);
		cb->addTimeParam(params, "Speed", 'sped',
			TimeUnit::samples, 2.0, 441000.0, 44100.0,
			TimeUnit::seconds | TimeUnit::samples | TimeUnit::ticks | TimeUnit::hertz, TimeUnit::seconds);
		cb->addRealParam(params, "Send step", 'step', 1, 256, 16, ParamFlags::integer);

		return true;
	}

	Lfo(HostMachine* mac, Callbacks* cb) : Mi(mac,cb), m_phase(0), m_centre(0.5), m_amp(0.5),
		m_sendphase(0), m_sendphasemax(32)
	{
		setPeriod(1.0);
	}

	void setPeriod(double period)
	{
		m_phasestep = 2.0 * M_PI / period;
	}

	void changeParam(ParamTag tag, ParamValue value)
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
				m_cb->addParamChange(&outpins[0], i, m_centre + sin(m_phase) * m_amp);
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

void CosecantAPI::populateMiFactories(std::map<std::string, MiFactory*>& factories)
{
	factories["btdsys/lfo"] = new MiFactory_T<Lfo>;
}
