#include "stdafx.h"
#include "common.h"
#include "builtinmachines.h"
#include "sequence.h"
//#include "notebookwindow.h"
#include "delayline.h"

#include "builtinmachines_audioio.h"
#include "spatternmachine.h"

///////////////////////////////////////////////////////////////////////////

void initBuiltinMachineFactories()
{
#	define INIT_FACTORY(type, id, desc)  MachineFactory::add(id, new BuiltinMachineFactory<type>(id, desc));
	INIT_FACTORY(Builtin::AudioOut,		"builtin/aout",		"Audio Out");
	INIT_FACTORY(Builtin::AudioIn,		"builtin/ain",		"Audio In");
	INIT_FACTORY(SpatternMachine,		"builtin/spattern",	"Standard Pattern");
	INIT_FACTORY(Builtin::Dummy,		"builtin/dummy",	"Dummy");
#	undef INIT_FACTORY
}
