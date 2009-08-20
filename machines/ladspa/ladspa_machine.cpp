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

LadspaMachine::LadspaMachine(HostMachine* hm, const std::wstring& dllname, int index)
: Mi(hm), m_index(index), m_handle(NULL), m_infoIsInited(false)
{
	m_dll.init(dllname.c_str());
	// If it throws an exception, let it propagate out
}

void LadspaMachine::init()
{
	if (!m_infoIsInited) initInfo();

	m_handle = m_desc->instantiate(m_desc, 44100);
	if (!m_handle)
		throw LadspaError("instantiate returned NULL");

	for (unsigned long port=0; port < m_desc->PortCount; port++)
	{
		m_desc->connect_port(m_handle, port, &m_portBuffers[port].front());
	}

	if (m_desc->activate) m_desc->activate(m_handle);
}

MachineInfo* LadspaMachine::getInfo()
{
	if (!m_infoIsInited) initInfo();
	return &m_info;
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

void LadspaMachine::work(const PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe)
{
	// Copy input buffers
	for (size_t i=0; i<m_inPinBuffers.size(); i++)
	{
		const PinBufInfo& pbi = m_inPinBuffers[i];
		if (pbi.bufR) // stereo
		{
			for (int j=0; j<lastframe-firstframe; j++)
			{
				(*pbi.bufL)[j] = inpins[i].f[(firstframe+j)*2];
				(*pbi.bufR)[j] = inpins[i].f[(firstframe+j)*2+1];
			}
		}
		else // mono
		{
			for (int j=0; j<lastframe-firstframe; j++)
			{
				(*pbi.bufL)[j] = inpins[i].f[firstframe+j];
			}
		}
	}

	// Do the work
	m_desc->run(m_handle, lastframe-firstframe);

	// Copy output buffers
	for (size_t i=0; i<m_outPinBuffers.size(); i++)
	{
		const PinBufInfo& pbi = m_outPinBuffers[i];
		if (pbi.isControl)
		{
			g_host->addParamChangeEvent(&outpins[i], firstframe, pbi.bufL->front());
		}
		else
		{
			if (pbi.bufR) // stereo
			{
				for (int j=0; j<lastframe-firstframe; j++)
				{
					outpins[i].f[(firstframe+j)*2  ] = (*pbi.bufL)[j];
					outpins[i].f[(firstframe+j)*2+1] = (*pbi.bufR)[j];
				}
			}
			else // mono
			{
				for (int j=0; j<lastframe-firstframe; j++)
				{
					outpins[i].f[firstframe+j] = (*pbi.bufL)[j];
				}
			}
		}
	}
}

void LadspaMachine::changeParam(ParamTag tag, double value)
{
	std::map<ParamTag, Buffer*>::iterator iter = m_paramBuffers.find(tag);
	if (iter != m_paramBuffers.end())
		iter->second->front() = (float)value;
}
