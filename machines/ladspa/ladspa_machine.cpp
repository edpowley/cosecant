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

void LadspaMachine::work(const WorkContext* ctx)
{
	// Process events
	ESIter enditer = ESIter::upperBound(ctx->ev, ctx->firstframe);
	for (Helper::ESIter iter = ESIter::lowerBound(ctx->ev, ctx->firstframe); iter != enditer; ++iter)
	{
		StreamEvent ev = iter.value();
		switch (ev.type)
		{
		case StreamEventType::paramChange:
			{
				std::map<ParamTag, Buffer*>::iterator paramIter = m_paramBuffers.find(ev.paramChange.tag);
				if (paramIter != m_paramBuffers.end())
					paramIter->second->front() = (float)ev.paramChange.value;
			}
			break;
		}
	}

	// Copy input buffers
	for (size_t i=0; i<m_inPinBuffers.size(); i++)
	{
		const PinBufInfo& pbi = m_inPinBuffers[i];
		if (pbi.bufR) // stereo
		{
			for (int j=0; j<ctx->lastframe-ctx->firstframe; j++)
			{
				(*pbi.bufL)[j] = ctx->in[i].f[(ctx->firstframe+j)*2];
				(*pbi.bufR)[j] = ctx->in[i].f[(ctx->firstframe+j)*2+1];
			}
		}
		else // mono
		{
			for (int j=0; j<ctx->lastframe-ctx->firstframe; j++)
			{
				(*pbi.bufL)[j] = ctx->in[i].f[ctx->firstframe+j];
			}
		}
	}

	// Do the work
	m_desc->run(m_handle, ctx->lastframe-ctx->firstframe);

	// Copy output buffers
	for (size_t i=0; i<m_outPinBuffers.size(); i++)
	{
		const PinBufInfo& pbi = m_outPinBuffers[i];
		if (pbi.isControl)
		{
			g_host->addParamChangeEvent(&ctx->out[i], ctx->firstframe, pbi.bufL->front());
		}
		else
		{
			if (pbi.bufR) // stereo
			{
				for (int j=0; j<ctx->lastframe-ctx->firstframe; j++)
				{
					ctx->out[i].f[(ctx->firstframe+j)*2  ] = (*pbi.bufL)[j];
					ctx->out[i].f[(ctx->firstframe+j)*2+1] = (*pbi.bufR)[j];
				}
			}
			else // mono
			{
				for (int j=0; j<ctx->lastframe-ctx->firstframe; j++)
				{
					ctx->out[i].f[ctx->firstframe+j] = (*pbi.bufL)[j];
				}
			}
		}
	}
}
