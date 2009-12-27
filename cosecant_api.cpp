#include "stdafx.h"
#include "cosecant_api.h"

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
	void Mi_destroy(Mi* m)
	{ delete m; }
		
	MachineInfo* Mi_getInfo(Mi* m)
	{ return m->getInfo(); }

	void Mi_init(Mi* m)
	{ m->init(); }
		
	void Mi_work(Mi* m, const WorkContext* ctx)
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


