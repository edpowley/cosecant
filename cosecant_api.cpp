#include "stdafx.h"
#include "cosecant_api.h"
using namespace CosecantAPI;

#define DLLEXPORT(RETURN_TYPE) extern "C" __declspec(dllexport) RETURN_TYPE

////////////////////////////////////////////////////////////////////////////

class MachineFactories
{
public:
	static MachineFactories singleton;
	std::map<std::string, MiFactory*> m_fac;

private:
	MachineFactories()
	{
		CosecantAPI::populateMiFactories(m_fac);
	}
};

MachineFactories MachineFactories::singleton;

////////////////////////////////////////////////////////////////////////////

DLLEXPORT(void) getMachineIds(void (*cb)(void*, const char*), void* cbArg)
{
	for (std::map<std::string, MiFactory*>::const_iterator
		iter  = MachineFactories::singleton.m_fac.begin();
		iter != MachineFactories::singleton.m_fac.end();
		++ iter)
	{
		cb(cbArg, iter->first.c_str());
	}
}

////////////////////////////////////////////////////////////////////////////

DLLEXPORT(bool) getInfo(MachineInfo* info, InfoCallbacks* cb, const char* id)
{
	std::map<std::string, MiFactory*>::const_iterator
		iter  = MachineFactories::singleton.m_fac.find(id);
	if (iter != MachineFactories::singleton.m_fac.end())
		return iter->second->getInfo(info, cb);
	else
		return false;
}

////////////////////////////////////////////////////////////////////////////

DLLEXPORT(Mi*) createMachine(const char* id, HostMachine* mac, Callbacks* cb)
{
	std::map<std::string, MiFactory*>::const_iterator
		iter  = MachineFactories::singleton.m_fac.find(id);
	if (iter != MachineFactories::singleton.m_fac.end())
		return iter->second->createMachine(mac,cb);
	else
		return NULL;
}
