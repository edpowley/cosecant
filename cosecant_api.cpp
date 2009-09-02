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

	MiPattern* Mi_createPattern(Mi* m, double length)
	{ return m->createPattern(length); }

	void MiPattern_destroy(MiPattern* p)
	{ delete p; }
};

static CosecantAPI::PluginFunctions g_pluginFuncs = {
	CosecantPlugin::enumerateFactories,
	CosecantPlugin::createMachine,
	PluginFuncImpl::Mi_destroy,
	PluginFuncImpl::Mi_getInfo,
	PluginFuncImpl::Mi_init,
	PluginFuncImpl::Mi_work,
	PluginFuncImpl::Mi_createPattern,
	PluginFuncImpl::MiPattern_destroy,
};

COSECANT_EXPORT(CosecantAPI::PluginFunctions*) csc_getPluginFunctions()
{
	return &g_pluginFuncs;
}


