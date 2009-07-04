#pragma once

#include "machine.h"

template<class MiClass>
class BuiltinMachine : public Machine
{
public:
	virtual Mi* createMi(Callbacks* cb)
	{
		m_mi = new MiClass(this, cb);
		return m_mi;
	}
};

template<class MiClass>
class BuiltinMachineFactory : public MachineFactory
{
public:
	virtual InfoImpl::MachineInfo* getMachInfo()
	{
		InfoImpl::MachineInfo* info = new InfoImpl::MachineInfo;
		InfoImpl::InfoCallbacks cb;
		MiClass::getInfo(info, &cb); // TODO: check return value
		return info;
	}

protected:
	virtual Ptr<Machine> createMachineImpl()
	{
		return new BuiltinMachine<MiClass>();
	}
};

void initBuiltinMachineFactories();
