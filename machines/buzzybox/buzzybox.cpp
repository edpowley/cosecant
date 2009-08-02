// buzzybox.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "buzzybox.h"

enum MacKey
{
	mk_gain,
	mk_mm2s,
	mk_s2mm,
};

//////////////////////////////////////////////////////////////////////

static void registerFactory(MiFactoryList* list, const char* id, const char* desc, MacKey key)
{
	g_host->registerMiFactory(list, id, desc, &key, sizeof(key));
}

void CosecantPlugin::enumerateFactories(MiFactoryList* list)
{
	registerFactory(list, "btdsys/buzzybox/gain", "BuzzyBox Gain", mk_gain);
	registerFactory(list, "btdsys/buzzybox/mm2s", "BuzzyBox Stereo Join", mk_mm2s);
	registerFactory(list, "btdsys/buzzybox/s2mm", "BuzzyBox Stereo Split", mk_s2mm);
}

Mi* CosecantPlugin::createMachine(const void* facUser, unsigned int facUserSize, HostMachine* hm)
{
	MacKey key = *reinterpret_cast<const MacKey*>(facUser);
	switch (key)
	{
	case mk_gain:
		return reinterpret_cast<Mi*>(new Gain(hm));
	case mk_mm2s:
		return reinterpret_cast<Mi*>(new MonoMonoToStereo(hm));
	case mk_s2mm:
		return reinterpret_cast<Mi*>(new StereoToMonoMono(hm));
	default:
		return NULL;
	}
}
