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
	BuiltinMachineFactory(const QString& desc) : m_desc(desc) {}

	virtual QString getDesc() { return m_desc; }

protected:
	QString m_desc;

	virtual Ptr<Machine> createMachineImpl()
	{
		return new MachineClass();
	}
};

void initBuiltinMachineFactories();
