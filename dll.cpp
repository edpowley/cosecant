#include "stdafx.h"
#include "common.h"
#include "dll.h"

bool Dll::isDll(const bpath& path)
{
	return _wcsicmp(path.extension().c_str(), L".dll") == 0;
}

Dll::Dll(const bpath& path)
{
	m_hmod = LoadLibrary(path.file_string().c_str());
	if (!m_hmod)
	{
		THROW_ERROR(InitError,
			QString("LoadLibrary failed, error code %1").arg(GetLastError()));
	}
}

Dll::~Dll()
{
	if (m_hmod)
		FreeLibrary(m_hmod);
}

void* Dll::getFunc(const char* name)
{
	return GetProcAddress(m_hmod, name);
}
