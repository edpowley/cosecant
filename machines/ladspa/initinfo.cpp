#include "stdafx.h"
#include "ladspa_machine.h"

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

void LadspaMachine::initInfo()
{
	m_infoIsInited = true;

	m_desc = m_dll.callDescriptorFunc(m_index);
	if (!m_desc)
		throw LadspaError("ladspa_descriptor returned NULL");

	m_portBuffers.resize(m_desc->PortCount);

	m_info.defaultName = m_desc->Name;

	bool hasAudioInput = false;
	bool hasAudioOutput = false;

	for (unsigned long port=0; port < m_desc->PortCount; port++)
	{
		LADSPA_PortDescriptor pdesc = m_desc->PortDescriptors[port];
		if (LADSPA_IS_PORT_INPUT(pdesc) && LADSPA_IS_PORT_CONTROL(pdesc))
		{
			addParam(port);
			m_portBuffers[port].resize(1, 0.0f);
			m_paramBuffers[ptPluginFirst + port] = &m_portBuffers[port];
		}
		else if (LADSPA_IS_PORT_OUTPUT(pdesc) && LADSPA_IS_PORT_CONTROL(pdesc))
		{
			m_pins.push_back(PinInfo());
			PinInfo& pi = m_pins.back();
			pi.name = m_desc->PortNames[port];
			pi.type = SignalType::paramControl;
			m_outPinPtrs.push_back(&pi);

			m_portBuffers[port].resize(1, 0.0f);
			m_outPinBuffers.push_back(PinBufInfo(&m_portBuffers[port], NULL, true));
		}
		else if (LADSPA_IS_PORT_AUDIO(pdesc))
		{
			bool stereo = portIsFirstOfStereoPair(port);
			m_pins.push_back(PinInfo());
			PinInfo& pi = m_pins.back();
			pi.name = m_desc->PortNames[port];
			pi.type = stereo ? SignalType::stereoAudio : SignalType::monoAudio;

			m_portBuffers[port].resize(maxFramesPerBuffer, 0.0f);
			PinBufInfo pbi(&m_portBuffers[port], NULL, false);
			if (stereo)
			{
				m_portBuffers[port+1].resize(maxFramesPerBuffer, 0.0f);
				pbi.bufR = &m_portBuffers[port+1];
			}

			if (LADSPA_IS_PORT_INPUT(pdesc))
			{
				m_inPinPtrs.push_back(&pi);
				m_inPinBuffers.push_back(pbi);
				hasAudioInput = true;
			}
			else
			{
				m_outPinPtrs.push_back(&pi);
				m_outPinBuffers.push_back(pbi);
				hasAudioOutput = true;
			}

			if (stereo) port++; // skip the next one
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
}

void LadspaMachine::addParam(unsigned long port)
{
	const LADSPA_PortRangeHint* phint = &m_desc->PortRangeHints[port];
	LADSPA_PortRangeHintDescriptor h = phint->HintDescriptor;

	double min = phint->LowerBound;
	double max = phint->UpperBound;
	double def = calcDefValue(min, max, h);

	const char* pname = m_desc->PortNames[port];
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

bool LadspaMachine::portIsFirstOfStereoPair(unsigned long port)
{
	// Last port can't be first of a stereo pair
	if (port+1 >= m_desc->PortCount) return false;

	LADSPA_PortDescriptor pdescL = m_desc->PortDescriptors[port];
	LADSPA_PortDescriptor pdescR = m_desc->PortDescriptors[port+1];

	// Both must be audio ports
	if (!LADSPA_IS_PORT_AUDIO(pdescL) || !LADSPA_IS_PORT_AUDIO(pdescR))
		return false;

	// Both must be input or output
	if (LADSPA_IS_PORT_INPUT(pdescL) & LADSPA_IS_PORT_OUTPUT(pdescR))
		return false;
	if (LADSPA_IS_PORT_OUTPUT(pdescL) & LADSPA_IS_PORT_INPUT(pdescR))
		return false;

	// Get lowercase names
	std::string nameL(m_desc->PortNames[port]);
	std::transform(nameL.begin(), nameL.end(), nameL.begin(), tolower);
	std::string nameR(m_desc->PortNames[port+1]);
	std::transform(nameR.begin(), nameR.end(), nameR.begin(), tolower);

	return portIsFirstOfStereoPair_ByName(nameL, nameR, "left", "right")
		|| portIsFirstOfStereoPair_ByName(nameL, nameR, "l", "r");
}

bool LadspaMachine::portIsFirstOfStereoPair_ByName(const std::string& nameL, const std::string& nameR,
												   const std::string& lstr, const std::string& rstr)
{
	size_t pos = 0;
	while (pos != std::string::npos)
	{
		pos = nameL.find(lstr, pos);
		if (pos == std::string::npos) break;

		std::string foo(nameL); foo.replace(pos, lstr.length(), rstr);
		if (foo == nameR) return true;
	}

	return false;
}
