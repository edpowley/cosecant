#pragma once

#include "machine.h"
#include "dll.h"

namespace DllMachine
{

	class Factory : public MachineFactory
	{
	public:
		Factory(const QString& dllpath, const QString& id)
			: m_dllpath(dllpath), m_id(id) {}

		virtual InfoImpl::MachineInfo* getMachInfo();

		static void scan(const QString& path);

	protected:
		QString m_id;
		QString m_dllpath;

		virtual Ptr<Machine> createMachineImpl();
	};

	class Mac : public Machine
	{
	public:
		Mac(const QString& dllpath, const QString& id);
		virtual ~Mac();

		virtual Mi* createMi(Callbacks* cb);

	protected:
		Ptr<Dll> m_dll;
		QString m_id;
	};

};
