#include "stdafx.h"
#include "common.h"
#include "builtinmachines.h"
#include "sequence.h"
//#include "notebookwindow.h"
#include "delayline.h"

#include "builtinmachines_audioio.h"

namespace Builtin
{
	class Dummy : public Machine
	{
	public:
		Dummy()
		{
			m_dead = true;
			m_deadWhy = QCoreApplication::translate("RoutingEditor::Editor",
				"This is a placeholder for a machine which you do not have installed." );
		}

		virtual void changeParam(ParamTag tag, double value) {}
		virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe) {}

	protected:
		virtual void initInfo()
		{
			static MachineInfo info;
			info.defaultName = "Dummy";
			m_info = &info;
		}
	
		virtual void initImpl() {}
	};

}; // end namespace Builtin

///////////////////////////////////////////////////////////////////////////

void initBuiltinMachineFactories()
{
	MachineFactory::add("builtin/aout",		new BuiltinMachineFactory<Builtin::AudioOut>);
	MachineFactory::add("builtin/ain",		new BuiltinMachineFactory<Builtin::AudioIn>);
	MachineFactory::add("builtin/dummy",	new BuiltinMachineFactory<Builtin::Dummy>);
}
