// ladspa.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "ladspa_machine.h"

void scanDll(MiFactoryList* list, const std::wstring& filename)
{
	LadspaDll dll;
	try
	{
		dll.init(filename.c_str());
	}
	catch (const LadspaError&)
	{
		return;
	}

	for (int index=0; true; index++) // loop ends when callDescriptorFunc returns NULL
	{
		const LADSPA_Descriptor* desc = dll.callDescriptorFunc(index);
		if (!desc) break;

		Blob data;
		data.push(index);
		data.pushString(filename);

		std::ostringstream cscId;
		cscId << "btdsys/ladspa/" << desc->UniqueID;

		g_host->registerMiFactory(list,
			cscId.str().c_str(),
			desc->Name,
			data.getData(),
			data.getDataSize()
		);
	}
}

struct ScanDirData
{
	MiFactoryList* list;
};

void scanDir(void* user, const PathChar* pathsz)
{
	StatusMessage status(StatusMessageBuilder() << "Scanning " << toUtf8(pathsz));

	ScanDirData* data = reinterpret_cast<ScanDirData*>(user);

	std::wstring path = pathsz;
	if (path[path.length()-1] != '\\') path += L"\\";

	WIN32_FIND_DATA finddata;
	HANDLE hfind = FindFirstFile( (path + L"*.dll").c_str(), &finddata );
	if (hfind != INVALID_HANDLE_VALUE)
	{
		do
		{
			scanDll(data->list, path + finddata.cFileName);
		}
		while (FindNextFile(hfind, &finddata));
		FindClose(hfind);
	}
}

void CosecantPlugin::enumerateFactories(MiFactoryList* list)
{
	StatusMessage status(StatusMessageBuilder() << "Scanning LADSPA plugins");

	ScanDirData data;
	data.list = list;
	g_host->iteratePaths("btdsys/ladspa", "LADSPA plugins", scanDir, &data);

	g_host->popStatus();
}

Mi* CosecantPlugin::createMachine(const void* facUser, unsigned int facUserSize, HostMachine* hm)
{
	try
	{
		Blob data(facUser, facUserSize);
		int index = data.pull<int>();
		std::wstring dllname = data.pullString<wchar_t>();

		return new LadspaMachine(hm, dllname, index);
	}
	catch (const Blob::BlobEmpty&)
	{
		return NULL;
	}
	catch (const LadspaError& err)
	{
		DebugPrint() << err.what();
		return NULL;
	}
}
