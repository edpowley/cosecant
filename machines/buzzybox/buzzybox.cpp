// buzzybox.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "buzzybox.h"

void CosecantAPI::populateMiFactories(std::map<std::string, MiFactory*>& factories)
{
#define BUZZYBOX_INIT_FACTORY(classname) \
	factories["btdsys/buzzybox/" #classname] = new MiFactory_T<classname>
// end define

	BUZZYBOX_INIT_FACTORY(Gain);
	BUZZYBOX_INIT_FACTORY(MonoMonoToStereo);
	BUZZYBOX_INIT_FACTORY(StereoToMonoMono);

#undef BUZZYBOX_INIT_FACTORY
}
