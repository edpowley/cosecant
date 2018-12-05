#include "stdafx.h"
#include "cosecant_api.h"

#ifndef COSECANT_API_HOST

COSECANT_EXPORT(uint32_t) csc_getVersion()
{
	return CosecantAPI::version;
}

CosecantAPI::HostFunctions* CosecantAPI::g_host;

COSECANT_EXPORT(void) csc_setHostFunctions(CosecantAPI::HostFunctions* host)
{
	CosecantAPI::g_host = host;
}

//////////////////////////////////////////////////////////////////////////////////

namespace PluginFuncImpl
{
    void Mi_destroy(CosecantAPI::Mi* m)
	{ delete m; }
		
    CosecantAPI::MachineInfo* Mi_getInfo(CosecantAPI::Mi* m)
	{ return m->getInfo(); }

    void Mi_init(CosecantAPI::Mi* m)
	{ m->init(); }
		
    void Mi_work(CosecantAPI::Mi* m, const CosecantAPI::WorkContext* ctx)
	{ m->work(ctx); }
};

static CosecantAPI::PluginFunctions g_pluginFuncs = {
	CosecantPlugin::enumerateFactories,
	CosecantPlugin::createMachine,
	PluginFuncImpl::Mi_destroy,
	PluginFuncImpl::Mi_getInfo,
	PluginFuncImpl::Mi_init,
	PluginFuncImpl::Mi_work,
};

COSECANT_EXPORT(CosecantAPI::PluginFunctions*) csc_getPluginFunctions()
{
	return &g_pluginFuncs;
}

#endif
