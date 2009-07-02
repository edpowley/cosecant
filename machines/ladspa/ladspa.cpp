// ladspa.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ladspa_machine.h"

void scanDll(std::map<std::string, MiFactory*>& factories, const std::wstring& filename)
{
	LadspaDll dll;
	try
	{
		dll.init(filename.c_str());
	}
	catch (const LadspaError&)
	{
		return;
	}

	for (int index=0; true; index++)
	{
		const LADSPA_Descriptor* desc = dll.callDescriptorFunc(index);
		if (!desc) break;

		LadspaMachineFactory* fac = new LadspaMachineFactory(filename, index, desc);

		std::ostringstream cscId;
		cscId << "btdsys/ladspa/" << desc->UniqueID;

		factories[cscId.str()] = fac;
	}
}

void CosecantAPI::populateMiFactories(std::map<std::string, MiFactory*>& factories)
{
	std::wstring basepath = L"C:\\Program Files\\Audacity\\Plug-Ins";
	if (basepath[basepath.length()-1] != '\\') basepath += L"\\";

	WIN32_FIND_DATA finddata;
	HANDLE hfind = FindFirstFile((basepath + L"*.dll").c_str(), &finddata);
	if (hfind != INVALID_HANDLE_VALUE)
	{
		do
		{
			scanDll(factories, basepath + finddata.cFileName);
		}
		while (FindNextFile(hfind, &finddata));
		FindClose(hfind);
	}
}

/////////////////////////////////////////////////////////////////////////

bool LadspaMachineFactory::getInfo(MachineInfo* info, const InfoCallbacks* cb)
{
	LadspaDll dll;
	try
	{
		dll.init(m_dllname.c_str());
	}
	catch (const LadspaError&)
	{
		return false;
	}

	const LADSPA_Descriptor* desc = dll.callDescriptorFunc(m_index);
	if (!desc) return false;

	cb->setName(info, desc->Name);
	ParamGroup* params = cb->createParamGroup("", 0);
	cb->setParams(info, params);

	bool hasAudioInput = false;
	bool hasAudioOutput = false;

	for (unsigned long port=0; port < desc->PortCount; port++)
	{
		LADSPA_PortDescriptor pdesc = desc->PortDescriptors[port];
		const char* pname = desc->PortNames[port];
		if (LADSPA_IS_PORT_INPUT(pdesc) && LADSPA_IS_PORT_CONTROL(pdesc))
		{
			const LADSPA_PortRangeHint* phint = &desc->PortRangeHints[port];
			LADSPA_PortRangeHintDescriptor h = phint->HintDescriptor;

			double min = phint->LowerBound;
			double max = phint->UpperBound;
			if (LADSPA_IS_HINT_SAMPLE_RATE(h))
			{
				min *= 44100.0;
				max *= 44100.0;
			}
		
			double def = min;
			if (LADSPA_IS_HINT_HAS_DEFAULT(h))
			{
				if (LADSPA_IS_HINT_DEFAULT_MINIMUM(h))
					def = min;
				else if (LADSPA_IS_HINT_DEFAULT_LOW(h))
					def = min * 0.75 + max * 0.25;
				else if (LADSPA_IS_HINT_DEFAULT_MIDDLE(h))
					def = (min + max) * 0.5;
				else if (LADSPA_IS_HINT_DEFAULT_HIGH(h))
					def = min * 0.25 + max * 0.75;
				else if (LADSPA_IS_HINT_DEFAULT_MAXIMUM(h))
					def = max;
				else if (LADSPA_IS_HINT_DEFAULT_0(h))
					def = 0.0;
				else if (LADSPA_IS_HINT_DEFAULT_1(h))
					def = 1.0;
				else if (LADSPA_IS_HINT_DEFAULT_100(h))
					def = 100.0;
				else if (LADSPA_IS_HINT_DEFAULT_440(h))
					def = 440.0;
			}

			ParamTag tag = 0x10000 + port;

			unsigned long flags = 0;
			if (LADSPA_IS_HINT_INTEGER(h))		flags |= ParamFlags::integer;
			if (LADSPA_IS_HINT_LOGARITHMIC(h))	flags |= ParamFlags::logarithmic;
			cb->addRealParam(params, pname, tag, min, max, def, flags);
		}
		else if (LADSPA_IS_PORT_OUTPUT(pdesc) && LADSPA_IS_PORT_CONTROL(pdesc))
		{
			cb->addOutPin(info, pname, SignalType::paramControl);
		}
		else if (LADSPA_IS_PORT_AUDIO(pdesc))
		{
			if (LADSPA_IS_PORT_INPUT(pdesc))
			{
				cb->addInPin(info, pname, SignalType::monoAudio);
				hasAudioInput = true;
			}
			else
			{
				cb->addOutPin(info, pname, SignalType::monoAudio);
				hasAudioOutput = true;
			}
		}
	}

	if (hasAudioInput)
		cb->setTypeHint(info, MachineTypeHint::effect);
	else if (hasAudioOutput)
		cb->setTypeHint(info, MachineTypeHint::generator);
	else
		cb->setTypeHint(info, MachineTypeHint::control);

	return true;
}