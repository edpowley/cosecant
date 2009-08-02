#pragma once

#include "machine.h"

class BuiltinMachine : public Machine
{
public:
};

template<class MachineClass>
class BuiltinMachineFactory : public MachineFactory
{
public:

protected:
	virtual Ptr<Machine> createMachineImpl()
	{
		return new MachineClass();
	}
};

void initBuiltinMachineFactories();
