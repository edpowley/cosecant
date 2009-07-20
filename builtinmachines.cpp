#include "stdafx.h"
#include "common.h"
#include "builtinmachines.h"
#include "machinfo.h"
#include "sequence.h"
//#include "notebookwindow.h"
#include "delayline.h"

#include "builtinmachines_audioio.h"

namespace Builtin
{
	class Dummy : public Mi
	{
	public:
		Dummy(Callbacks* cb) : Mi(cb)
		{
			m_cb->getHostMachine()->m_dead = true;
			m_cb->getHostMachine()->m_deadWhy 
				= QCoreApplication::translate("RoutingEditor::Editor",
				"This is a placeholder for a machine which you do not have installed." );
		}

		virtual void changeParam(ParamTag tag, ParamValue value) {}
		virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe) {}

		static bool getInfo(InfoImpl::MachineInfo* info, InfoImpl::InfoCallbacks* cb)
		{
			info->setName("Dummy")->setTypeHint(MachineTypeHint::none);
			return true;
		}
	};

}; // end namespace Builtin

///////////////////////////////////////////////////////////////////////////

void initBuiltinMachineFactories()
{
	MachineFactory::add("builtin/aout",		new BuiltinMachineFactory<Builtin::AudioOut>);
	MachineFactory::add("builtin/ain",		new BuiltinMachineFactory<Builtin::AudioIn>);
	MachineFactory::add("builtin/dummy",	new BuiltinMachineFactory<Builtin::Dummy>);
}
