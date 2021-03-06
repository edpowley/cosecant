#pragma once

#include "htmlentity.h"

//#define COSECANT_API_NO_CLASSES

inline QString decodeApiString(const char* str)
{
	return convertHtmlEntitiesToUnicode(QString::fromUtf8(str));
}

class MachineFactory;
class Machine;
namespace WorkBuffer { class Base; };

namespace CosecantAPI
{
	class MiFactoryList
	{
	public:
		MiFactoryList(const QString& dllpath) : m_dllpath(dllpath) {}

		QString m_dllpath;
	};

	typedef Machine HostMachine;
	
	typedef WorkBuffer::Base HostPinBuffer;

	class EventStreamIter; // defined in callbacks.cpp
};
