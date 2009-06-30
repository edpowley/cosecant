#include "stdafx.h"
#include "ladspa_machine.h"

LadspaDll::LadspaDll()
{
	m_hDll = NULL;
	m_descFunc = NULL;
}

void LadspaDll::init(const wchar_t *filename)
{
	m_hDll = LoadLibraryEx(filename, NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
	if (!m_hDll)
		throw LadspaError("Failed to load dll");

	m_descFunc = (LADSPA_Descriptor_Function)GetProcAddress(m_hDll, "ladspa_descriptor");
	if (!m_descFunc)
	{
		FreeLibrary(m_hDll);
		m_hDll = NULL;
		throw LadspaError("Couldn't find ladspa_descriptor function");
	}
}

LadspaDll::~LadspaDll()
{
	if (m_hDll) FreeLibrary(m_hDll);
}

const LADSPA_Descriptor* LadspaDll::callDescriptorFunc(unsigned long index)
{
	if (m_descFunc)
		return m_descFunc(index);
	else
		throw LadspaError("You must call LadspaDll::init() first");
}

//////////////////////////////////////////////////////////////////////////////////

LadspaMachine::LadspaMachine(HostMachine* mac, Callbacks* cb, const std::wstring& dllname, int index)
: Mi(mac,cb), m_handle(NULL)
{
	m_dll.init(dllname.c_str());
	// If it throws an exception, let it propagate out

	m_desc = m_dll.callDescriptorFunc(index);
	if (!m_desc)
		throw LadspaError("ladspa_descriptor returned NULL");

	m_handle = m_desc->instantiate(m_desc, 44100);
	if (!m_handle)
		throw LadspaError("instantiate returned NULL");

	m_portBuffers.resize(m_desc->PortCount);

	for (unsigned long port=0; port < m_desc->PortCount; port++)
	{
		LADSPA_PortDescriptor pdesc = m_desc->PortDescriptors[port];

		if (LADSPA_IS_PORT_CONTROL(pdesc))
			m_portBuffers[port].resize(1, 0.0f);
		else if (LADSPA_IS_PORT_AUDIO(pdesc))
			m_portBuffers[port].resize(maxFramesPerBuffer, 0.0f);

		float* bufAddr = &m_portBuffers[port][0];

		if (LADSPA_IS_PORT_INPUT(pdesc) && LADSPA_IS_PORT_CONTROL(pdesc))
		{
			m_paramBuffers[0x10000 + port] = bufAddr;
		}
		else if (LADSPA_IS_PORT_INPUT(pdesc))
		{
			m_inPinBuffers.push_back(bufAddr);
		}
		else if (LADSPA_IS_PORT_OUTPUT(pdesc))
		{
			m_outPinBuffers.push_back(bufAddr);
			m_outPinIsControl.push_back(!!LADSPA_IS_PORT_CONTROL(pdesc));
		}

		m_desc->connect_port(m_handle, port, bufAddr);
	}

	if (m_desc->activate) m_desc->activate(m_handle);
}

LadspaMachine::~LadspaMachine()
{
	if (m_handle)
	{
		if (m_desc->deactivate) m_desc->deactivate(m_handle);
		m_desc->cleanup(m_handle);
		m_handle = NULL;
	}
}

void LadspaMachine::work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe)
{
	// Copy input buffers
	for (size_t i=0; i<m_inPinBuffers.size(); i++)
	{
		for (int j=0; j<lastframe-firstframe; j++)
		{
			m_inPinBuffers[i][j] = inpins[i].f[firstframe+j];
		}
	}

	// Do the work
	m_desc->run(m_handle, lastframe-firstframe);

	// Copy output buffers
	for (size_t i=0; i<m_outPinBuffers.size(); i++)
	{
		if (m_outPinIsControl[i])
		{
			m_cb->addParamChange(&outpins[i], firstframe, m_outPinBuffers[i][0]);
		}
		else
		{
			for (int j=0; j<lastframe-firstframe; j++)
			{
				outpins[i].f[firstframe+j] = m_outPinBuffers[i][j];
			}
		}
	}
}

void LadspaMachine::changeParam(ParamTag tag, ParamValue value)
{
	std::map<ParamTag, float*>::iterator iter = m_paramBuffers.find(tag);
	if (iter != m_paramBuffers.end())
		iter->second[0] = (float)value;
}
