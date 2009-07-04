#pragma once

#include "machine.h"
#include "dll.h"

class DllMachineFactory : public MachineFactory
{
public:
	DllMachineFactory(const QString& dllpath, const QString& id)
		: m_dllpath(dllpath), m_id(id) {}

	virtual InfoImpl::MachineInfo* getMachInfo();

	static void scan(const QString& path);

protected:
	QString m_id;
	QString m_dllpath;

	virtual Ptr<Machine> createMachineImpl();
};

class DllMachine : public Machine
{
public:
	DllMachine(const QString& dllpath, const QString& id);
	virtual ~DllMachine();

	virtual Mi* createMi(Callbacks* cb);

protected:
	Ptr<Dll> m_dll;
	QString m_id;
};
