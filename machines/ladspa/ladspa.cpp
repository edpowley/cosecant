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

struct ScanDirData
{
	MiFactoryList* list;
};

void scanDir(void* user, const PathChar* pathsz)
{
	StatusMessage status(StatusMessageBuilder() << "Scanning " << toUtf8(pathsz));

	ScanDirData* data = reinterpret_cast<ScanDirData*>(user);

	std::wstring path = pathsz;
	if (path[path.length()-1] != '\\') path += L"\\";

	WIN32_FIND_DATA finddata;
	HANDLE hfind = FindFirstFile( (path + L"*.dll").c_str(), &finddata );
	if (hfind != INVALID_HANDLE_VALUE)
	{
		do
		{
			scanDll(data->list, path + finddata.cFileName);
		}
		while (FindNextFile(hfind, &finddata));
		FindClose(hfind);
	}
}

void CosecantPlugin::enumerateFactories(MiFactoryList* list)
{
	StatusMessage status(StatusMessageBuilder() << "Scanning LADSPA plugins");

	ScanDirData data;
	data.list = list;
	g_host->iteratePaths("btdsys/ladspa", "LADSPA plugins", scanDir, &data);

	g_host->popStatus();
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

static double interpolate(double min, double max, double p, bool logarithmic)
{
	if (!logarithmic)
		return (1-p)*min + p*max;
	else
		return exp(interpolate(log(min), log(max), p, false));
}

static double calcDefValue(double min, double max, LADSPA_PortRangeHintDescriptor h)
{
	if (!LADSPA_IS_HINT_HAS_DEFAULT(h)) return min;
	bool islog = !!LADSPA_IS_HINT_LOGARITHMIC(h);

	if (LADSPA_IS_HINT_DEFAULT_MINIMUM(h))
		return min;
	else if (LADSPA_IS_HINT_DEFAULT_LOW(h))
		return interpolate(min, max, 0.25, islog);
	else if (LADSPA_IS_HINT_DEFAULT_MIDDLE(h))
		return interpolate(min, max, 0.5, islog);
	else if (LADSPA_IS_HINT_DEFAULT_HIGH(h))
		return interpolate(min, max, 0.75, islog);
	else if (LADSPA_IS_HINT_DEFAULT_MAXIMUM(h))
		return max;
	else if (LADSPA_IS_HINT_DEFAULT_0(h))
		return 0.0;
	else if (LADSPA_IS_HINT_DEFAULT_1(h))
		return 1.0;
	else if (LADSPA_IS_HINT_DEFAULT_100(h))
		return 100.0;
	else if (LADSPA_IS_HINT_DEFAULT_440(h))
		return 440.0;
	else
		return min;
}

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
			double def = calcDefValue(min, max, h);

			ParamTag tag = ptPluginFirst + port;

			ParamScale::e scale = ParamScale::linear;
			if (LADSPA_IS_HINT_LOGARITHMIC(h)) scale = ParamScale::logarithmic;

			if (LADSPA_IS_HINT_SAMPLE_RATE(h))
			{
				DebugPrint() << min << max << def;

				using namespace TimeUnit;
				m_timeParams.push_back(TimeParamInfo());
				TimeParamInfo& pi = m_timeParams.back();
				pi.p.name = pname;
				pi.p.tag = tag;

				pi.min.set(min, fracfreq);
				pi.max.set(max, fracfreq);

				bool defInHz = LADSPA_IS_HINT_HAS_DEFAULT(h) && (
					LADSPA_IS_HINT_DEFAULT_1(h) || LADSPA_IS_HINT_DEFAULT_100(h) || LADSPA_IS_HINT_DEFAULT_440(h)
				);
				pi.def.set(def, defInHz ? hertz : fracfreq);

				pi.internalUnit = hertz;
				pi.displayUnits = hertz | notenum;
				pi.defaultDisplayUnit = hertz;
				m_paramPtrs.push_back(&pi.p);
			}
			else if (LADSPA_IS_HINT_INTEGER(h))
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
