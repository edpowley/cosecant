// sinetest.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"

class SineTest : public Mi
{
public:
	static bool getInfo(MachineInfo* info, const InfoCallbacks* cb)
	{
		cb->setName(info, "Sin");
		cb->setTypeHint(info, MachineTypeHint::generator);

		cb->addOutPin(info, "Output", SignalType::stereoAudio);

		ParamGroup* params = cb->createParamGroup("", 0);
		cb->setParams(info, params);
		cb->addTimeParam(params, "Frequency", 'freq', TimeUnit::hertz, 20.0, 2000.0, 440.0,
			TimeUnit::hertz | TimeUnit::notenum, TimeUnit::hertz);
		cb->addRealParam(params, "Volume",    'volu',  0.0,    1.0,   0.5, 0);
		ParamGroup* g = cb->createParamGroup("Subgroup test", 0);
		cb->addSubGroup(params, g);
		cb->addEnumParam(g, "Test enum", 'tint', "Hello\nWorld\nhow\nare\nyou", 3);

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
