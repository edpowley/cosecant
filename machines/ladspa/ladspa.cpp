// ladspa.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ladspa_machine.h"

void scanDll(MiFactoryList* list, const std::wstring& filename)
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

	for (int index=0; true; index++) // loop ends when callDescriptorFunc returns NULL
	{
		const LADSPA_Descriptor* desc = dll.callDescriptorFunc(index);
		if (!desc) break;

		Blob data;
		data.push(index);
		data.pushString(filename);

		std::ostringstream cscId;
		cscId << "btdsys/ladspa/" << desc->UniqueID;

		g_host->registerMiFactory(list,
			cscId.str().c_str(),
			desc->Name,
			data.getData(),
			data.getDataSize()
		);
	}
}

void CosecantPlugin::enumerateFactories(MiFactoryList* list)
{
	std::wstring basepath = L"C:\\Users\\Ed\\Documents\\ladspa_plugins";
	if (basepath[basepath.length()-1] != '\\') basepath += L"\\";

	WIN32_FIND_DATA finddata;
	HANDLE hfind = FindFirstFile((basepath + L"*.dll").c_str(), &finddata);
	if (hfind != INVALID_HANDLE_VALUE)
	{
		do
		{
			scanDll(list, basepath + finddata.cFileName);
		}
		while (FindNextFile(hfind, &finddata));
		FindClose(hfind);
	}
}

Mi* CosecantPlugin::createMachine(const void* facUser, unsigned int facUserSize, HostMachine* hm)
{
	try
	{
		Blob data(facUser, facUserSize);
		int index = data.pull<int>();
		std::wstring dllname = data.pullString<wchar_t>();

		return new LadspaMachine(hm, dllname, index);
	}
	catch (const Blob::BlobEmpty&)
	{
		return NULL;
	}
	catch (const LadspaError& err)
	{
		DebugPrint() << err.what();
		return NULL;
	}
}

/////////////////////////////////////////////////////////////////////////

MachineInfo* LadspaMachine::getInfo()
{
	m_info.defaultName = m_desc->Name;

	bool hasAudioInput = false;
	bool hasAudioOutput = false;

	for (unsigned long port=0; port < m_desc->PortCount; port++)
	{
		LADSPA_PortDescriptor pdesc = m_desc->PortDescriptors[port];
		const char* pname = m_desc->PortNames[port];
		if (LADSPA_IS_PORT_INPUT(pdesc) && LADSPA_IS_PORT_CONTROL(pdesc))
		{
			const LADSPA_PortRangeHint* phint = &m_desc->PortRangeHints[port];
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

			ParamScale::e scale = ParamScale::linear;
			if (LADSPA_IS_HINT_LOGARITHMIC(h)) scale = ParamScale::logarithmic;

			if (LADSPA_IS_HINT_INTEGER(h))
			{
				m_intParams.push_back(IntParamInfo());
				IntParamInfo& pi = m_intParams.back();
				pi.p.name = pname;
				pi.p.tag = tag;
				pi.min = (int)min;
				pi.max = (int)max;
				pi.def = (int)def;
				m_paramPtrs.push_back(&pi.p);
			}
			else
			{
				m_realParams.push_back(RealParamInfo());
				RealParamInfo& pi = m_realParams.back();
				pi.p.name = pname;
				pi.p.tag = tag;
				pi.scale = scale;
				pi.min = min;
				pi.max = max;
				pi.def = def;
				m_paramPtrs.push_back(&pi.p);
			}
		}
		else if (LADSPA_IS_PORT_OUTPUT(pdesc) && LADSPA_IS_PORT_CONTROL(pdesc))
		{
			m_pins.push_back(PinInfo());
			PinInfo& pi = m_pins.back();
			pi.name = pname;
			pi.type = SignalType::paramControl;
			m_outPinPtrs.push_back(&pi);
		}
		else if (LADSPA_IS_PORT_AUDIO(pdesc))
		{
			m_pins.push_back(PinInfo());
			PinInfo& pi = m_pins.back();
			pi.name = pname;
			pi.type = SignalType::monoAudio;

			if (LADSPA_IS_PORT_INPUT(pdesc))
			{
				m_inPinPtrs.push_back(&pi);
				hasAudioInput = true;
			}
			else
			{
				m_outPinPtrs.push_back(&pi);
				hasAudioOutput = true;
			}
		}
	}

	m_paramPtrs.push_back(NULL);
	m_info.params.params = const_cast<const ParamInfo**>(&m_paramPtrs.front());
	m_inPinPtrs.push_back(NULL);
	m_info.inPins = const_cast<const PinInfo**>(&m_inPinPtrs.front());
	m_outPinPtrs.push_back(NULL);
	m_info.outPins = const_cast<const PinInfo**>(&m_outPinPtrs.front());

	if (hasAudioInput)
		m_info.typeHint = MachineTypeHint::effect;
	else if (hasAudioOutput)
		m_info.typeHint = MachineTypeHint::generator;
	else
		m_info.typeHint = MachineTypeHint::control;

	return &m_info;
}
