#include "stdafx.h"
#include "cosecant_api.h"

COSECANT_EXPORT(unsigned int) csc_getVersion()
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
		
	void Mi_changeParam(Mi* m, ParamTag tag, double value)
	{ m->changeParam(tag, value); }

	void Mi_work(Mi* m, PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe)
	{ m->work(inpins, outpins, firstframe, lastframe); }

	ScriptValue* Mi_callScriptFunction(Mi* m, int id, const ScriptValue** args, int numargs)
	{ return m->callScriptFunction(id, args, numargs); }
	
	MiPattern* Mi_createPattern(Mi* m, double length)
	{ return m->createPattern(length); }

	void MiPattern_destroy(MiPattern* p)
	{ delete p; }

	void MiPattern_play(MiPattern* p, SequenceTrack* track, double startpos)
	{ p->play(track, startpos); }

	void MiPattern_stop(MiPattern* p, SequenceTrack* track)
	{ p->stop(track); }
};

static CosecantAPI::PluginFunctions g_pluginFuncs = {
	CosecantPlugin::enumerateFactories,
	CosecantPlugin::createMachine,
	PluginFuncImpl::Mi_destroy,
	PluginFuncImpl::Mi_getInfo,
	PluginFuncImpl::Mi_init,
	PluginFuncImpl::Mi_changeParam,
	PluginFuncImpl::Mi_work,
	PluginFuncImpl::Mi_callScriptFunction,
	PluginFuncImpl::Mi_createPattern,
	PluginFuncImpl::MiPattern_destroy,
	PluginFuncImpl::MiPattern_play,
	PluginFuncImpl::MiPattern_stop,
};

COSECANT_EXPORT(CosecantAPI::PluginFunctions*) csc_getPluginFunctions()
{
	return &g_pluginFuncs;
}


