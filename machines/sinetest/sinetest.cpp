// sinetest.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

class SineTest : public Mi
{
public:
	static bool getInfo(MachineInfo* info, InfoCallbacks* cb)
	{
		info->setName("Sin")->setTypeHint(MachineTypeHint::generator)
			->addOutPin(cb->createPin()->setName("Output")->setType(SignalType::stereoAudio));

		ParamInfo::Group* params = info->getParams();

		using namespace TimeUnit;

		params->addParam(cb->createTimeParam('freq')->setName("Frequency")
			->setRange(20, hertz, 2000, hertz)->setDefault(440, hertz)
			->setInternalUnit(hertz)->addDisplayUnits(hertz | notenum)->setDefaultDisplayUnit(hertz));
		params->addParam(cb->createRealParam('volu')->setName("Volume")->setRange(0,1)->setDefault(0.5));

		return true;
	}

	SineTest(HostMachine* mac, Callbacks* cb) : Mi(mac,cb), m_phase(0), m_amp(0.2)
	{
		setFreq(440.0);
	}

	void setFreq(double freq)
	{
		m_phasestep = 2.0 * M_PI * freq / 44100.0;
	}

	void changeParam(ParamTag tag, ParamValue value)
	{
		switch (tag)
		{
		case 'freq':
			setFreq(value);
			break;

		case 'volu':
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

protected:
	double m_phase, m_phasestep, m_amp;
};

void CosecantAPI::populateMiFactories(std::map<std::string, MiFactory*>& factories)
{
	factories["btdsys/test/sinedll"] = new MiFactory_T<SineTest>;
}
