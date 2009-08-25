#pragma once

#include "htmlentity.h"

#define COSECANT_API_NO_CLASSES

inline QString decodeApiString(const char* str)
{
	return convertHtmlEntitiesToUnicode(QString::fromUtf8(str));
}

class MachineFactory;
class Machine;
namespace Sequence { class Track; };
namespace WorkBuffer { class Base; class EventStream; };

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

	typedef WorkBuffer::Base HostPinBuffer;
	typedef WorkBuffer::EventStream HostPinBuffer_EventStream;

	class EventStreamIter; // defined in callbacks.cpp
};
