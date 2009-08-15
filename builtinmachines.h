#pragma once

#include "machine.h"

class BuiltinMachine : public Machine
{
public:
};

//////////////////////////////////////////////////////////////////////

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

};

////////////////////////////////////////////////////////////////////////////////

template<class MachineClass>
class BuiltinMachineFactory : public MachineFactory
{
public:
	BuiltinMachineFactory(const QString& id, const QString& desc) : MachineFactory(id), m_desc(desc) {}

	virtual QString getDesc() { return m_desc; }

protected:
	QString m_desc;

	virtual Ptr<Machine> createMachineImpl()
	{
		return new MachineClass();
	}
};

void initBuiltinMachineFactories();
