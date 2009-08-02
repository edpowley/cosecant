// lfo.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

class Lfo : public Mi
{
public:
	static bool getInfo(MachineInfo* info, InfoCallbacks* cb)
	{
		info->setName("LFO")->setTypeHint(MachineTypeHint::control);
		info->addOutPin(
			cb->createPin()->setName("Output")->setType(SignalType::paramControl) );

		ParamInfo::Group* params = info->getParams();

		using namespace TimeUnit;
		params->addParam(
			cb->createTimeParam('sped')->setName("Speed")
			->setRange(2, samples, 10, seconds)->setDefault(1, seconds)
			->setInternalUnit(samples)
			->addDisplayUnits(seconds | samples | beats | hertz)->setDefaultDisplayUnit(seconds)
		);

		params->addParam(
			cb->createIntParam('step')->setName("Send step")
			->setRange(1, 256)->setDefault(16)
		);

		return true;
	}

	Lfo(Callbacks* cb) : Mi(cb), m_phase(0), m_centre(0.5), m_amp(0.5),
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
