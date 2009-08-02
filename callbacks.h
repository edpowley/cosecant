#pragma once

#include "cosecant_api.h"
using namespace CosecantAPI;

namespace MachineExports
{
	typedef unsigned int (*csc_getVersion)();
	typedef void (*csc_setHostFunctions)(HostFunctions*);
	typedef CosecantAPI::PluginFunctions* (*csc_getPluginFunctions)();
};

QScriptValue variantToScriptValue(const Variant& var);
