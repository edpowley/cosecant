#include "stdafx.h"
#include "common.h"
#include "dll.h"

bool Dll::isDll(const QString& path)
{
	return path.endsWith(".dll", Qt::CaseInsensitive);
}

Dll::Dll(const QString& path)
{
	ASSERT(sizeof(wchar_t) == 2);

    m_hmod = LoadLibrary((LPCWSTR)QDir::toNativeSeparators(path).utf16());
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
