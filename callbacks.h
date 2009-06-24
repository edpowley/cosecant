#pragma once

#include "cosecant_api.h"

namespace InfoCallbacksImpl { extern CosecantAPI::InfoCallbacks g; }
namespace CallbacksImpl { extern CosecantAPI::Callbacks g; }

namespace MachineExports
{
	using namespace CosecantAPI;

	typedef unsigned int (*getAPIVersion)();
	typedef void (*getMachineIds)(void (*)(void*, const char*), void*);
	typedef bool (*getInfo)(MachineInfo*, const InfoCallbacks*, const char*);
	typedef Mi* (*createMachine)(const char* id, HostMachine* mac, Callbacks* cb);
	typedef MachineFunctions* (*getFunctions)();
};
