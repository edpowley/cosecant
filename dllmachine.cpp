#include "stdafx.h"
#include "common.h"
#include "dllmachine.h"
using namespace DllMachine;

#include "callbacks.h"

MacDll::MacDll(const QString& path) : Dll(path)
{
	MachineExports::csc_setHostFunctions setHostFunctions
		= (MachineExports::csc_setHostFunctions)getFunc("csc_setHostFunctions");
	
	if (setHostFunctions)
		setHostFunctions(g_host);

	MachineExports::csc_getPluginFunctions getPluginFunctions
		= (MachineExports::csc_getPluginFunctions)getFunc("csc_getPluginFunctions");
	
	if (getPluginFunctions)
		m_funcs = getPluginFunctions();
	else
		throw DllMachineError("csc_getPluginFunctions does not exist");

	if (!m_funcs) throw DllMachineError("csc_getPluginFunctions returned NULL");
}

//////////////////////////////////////////////////////////////////////////////////

Factory::Factory(const QString& dllpath, const QString& id, const QString& desc, void* user, unsigned int userSize)
: MachineFactory(id), m_dllpath(dllpath), m_id(id), m_desc(desc)
{
	if (user && userSize > 0)
	{
		m_userData.resize(userSize);
		memcpy(&m_userData.front(), user, userSize);
	}
}

Ptr<Machine> Factory::createMachineImpl()
{
	return new Mac(m_dllpath, m_userData);
}

////////////////////////////////////////////////////////////////////////////////

void Factory::scan(const QString& path)
{
	QDir dir(path);
	foreach(QString fname, dir.entryList(QStringList("*.dll"), QDir::Files))
	{
		QString fpath = path + "/" + fname;
		try
		{
			Ptr<MacDll> dll = new MacDll(fpath);

			if (dll->m_funcs->MiFactory_enumerate)
			{
				MiFactoryList mfl(fpath);
				dll->m_funcs->MiFactory_enumerate(&mfl);
			}
		}
		catch (const Dll::InitError& err)
		{
			qDebug() << "Failed to load dll " << fpath << ": " << err.msg();
		}
		catch (const DllMachineError& err)
		{
			qDebug() << "Failed to load dll " << fpath << ": " << err.msg();
		}
	}
}

//////////////////////////////////////////////////////////////////////

Mac::Mac(const QString& dllpath, const std::vector<char>& user)
: m_mi(NULL)
{
	m_dll = new MacDll(dllpath);

	if (m_dll->m_funcs->Mi_create)
	{
		if (user.empty())
			m_mi = m_dll->m_funcs->Mi_create(NULL, 0, this);
		else
			m_mi = m_dll->m_funcs->Mi_create(&user.front(), user.size(), this);
	}
}

Mac::~Mac()
{
	if (m_dll->m_funcs->Mi_destroy)
		m_dll->m_funcs->Mi_destroy(m_mi);
}

void Mac::initInfo()
{
	ASSERT(m_dll->m_funcs->Mi_getInfo);
	m_info = m_dll->m_funcs->Mi_getInfo(m_mi);
}

void Mac::initImpl()
{
	if (m_dll->m_funcs->Mi_init)
		m_dll->m_funcs->Mi_init(m_mi);
}

void Mac::work(const WorkContext* ctx)
{
	if (m_dll->m_funcs->Mi_work)
		m_dll->m_funcs->Mi_work(m_mi, ctx);
}

////////////////////////////////////////////////////////////////////////////////

