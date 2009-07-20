#include "stdafx.h"
#include "common.h"
#include "dllmachine.h"

#include "callbacks.h"

InfoImpl::MachineInfo* DllMachineFactory::getMachInfo()
{
	try
	{
		Dll dll(m_dllpath);

		MachineExports::getInfo f = (MachineExports::getInfo)dll.getFunc("getInfo");

		InfoImpl::MachineInfo* info = new InfoImpl::MachineInfo;
		InfoImpl::InfoCallbacks callbacks;
		f(info, &callbacks, m_id.toAscii());

		// TODO: error checking: if getFunc returns NULL or f returns false

		return info;
	}
	catch (const Dll::InitError& err)
	{
		printf("DllMachineFactory::getMachInfo error:\n%s\n", err.what());
		return NULL;
	}
}

Ptr<Machine> DllMachineFactory::createMachineImpl()
{
	return new DllMachine(m_dllpath, m_id);
}

////////////////////////////////////////////////////////////////////////////////

void DllMachineFactory::scan(const QString& path)
{
	QDir dir(path);
	foreach(QString fname, dir.entryList(QStringList("*.dll"), QDir::Files))
	{
		QString fpath = path + "/" + fname;
		try
		{
			Dll dll(fpath);

			MachineExports::getMachineIds f
				= (MachineExports::getMachineIds)dll.getFunc("getMachineIds");
			if (f)
			{
				struct Callback
				{
					const QString& m_fname;
					Callback(const QString& fname) : m_fname(fname) {}

					static void callback(void* vinst, const char* id)
					{
						Callback* inst = (Callback*)vinst;
						Ptr<DllMachineFactory> fac = new DllMachineFactory(inst->m_fname, id);
						MachineFactory::add(id, fac);
					}
				};

				Callback cb(fpath);
				f(Callback::callback, (void*)&cb);
			}
		}
		catch (const Dll::InitError& err)
		{
			qDebug() << "Failed to load dll " << fpath << ": " << err.msg();
		}
	}
}

//////////////////////////////////////////////////////////////////////

DllMachine::DllMachine(const QString& dllpath, const QString& id)
: m_id(id)
{
	m_dll = new Dll(dllpath);
}

DllMachine::~DllMachine()
{
	if (m_mi) delete m_mi;
}

Mi* DllMachine::createMi(Callbacks* cb)
{
	MachineExports::createMachine c = (MachineExports::createMachine)m_dll->getFunc("createMachine");
	m_mi = c(m_id.toAscii(), cb);
	return m_mi;
}
