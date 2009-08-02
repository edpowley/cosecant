#pragma once

#define COSECANT_API_NO_CLASSES

class MachineFactory;
class Machine;
namespace Sequence { class Track; };

namespace CosecantAPI
{
	class MiFactoryList
	{
	public:
		MiFactoryList(const QString& dllpath) : m_dllpath(dllpath) {}

		QString m_dllpath;
	};

	typedef Machine HostMachine;
	
	typedef QScriptValue ScriptValue;
	
	typedef Sequence::Track SequenceTrack;
};
