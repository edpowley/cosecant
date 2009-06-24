#pragma once

#include "machine.h"

template<class MachineClass>
class BuiltinMachineFactory : public MachineFactory
{
public:
	virtual Ptr<MachInfo> getMachInfo()
	{
		return MachineClass::getInfo();
	}

protected:
	virtual Ptr<Machine> createMachineImpl()
	{
		return new MachineClass();
	}
};

void initBuiltinMachineFactories();
