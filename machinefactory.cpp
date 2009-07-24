#include "stdafx.h"
#include "common.h"
#include "machine.h"
#include "callbacks.h"

std::map<QString, Ptr<MachineFactory> > MachineFactory::s_factories;

bool MachineFactory::add(const QString &id, MachineFactory* factory)
{
	// todo: check if id already exists

	s_factories[id] = factory;
	return true;
}

Ptr<MachineFactory> MachineFactory::get(const QString &id)
{
	std::map<QString, Ptr<MachineFactory> >::iterator i = s_factories.find(id);
	if (i != s_factories.end())
		return i->second;
	else
		THROW_ERROR(BadIdError, "No machine factory for this id");
}

Ptr<Machine> MachineFactory::createMachine()
{
	Ptr<Machine> machine = createMachineImpl();
	machine->createMi(new CallbacksImpl(machine)); // TODO: memory leak
	machine->init(getMachInfo());

	return machine;
}
