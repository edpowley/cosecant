#pragma once

#include "cosecant_api.h"
using namespace CosecantAPI;

class CallbacksImpl : public Callbacks
{
public:
	CallbacksImpl(const Ptr<Machine>& mac) : m_mac(mac) {}

	virtual unsigned int getHostVersion() { return CosecantAPI::version; }
	virtual HostMachine* getHostMachine() { return m_mac.c_ptr(); }

	virtual const TimeInfo* getTimeInfo();

	virtual void addParamChange(PinBuffer* buf, int time, ParamValue value);
	virtual void addNoteEvent  (PinBuffer* buf, int time, NoteEvent* ev);

protected:
	Ptr<Machine> m_mac;

	int returnString(const QString& s, char* buf, int buf_size);

	virtual bool lockMutex();
	virtual void unlockMutex();
};

namespace MachineExports
{
	using namespace CosecantAPI;

	typedef unsigned int (*getAPIVersion)();
	typedef void (*getMachineIds)(void (*)(void*, const char*), void*);
	typedef bool (*getInfo)(MachineInfo*, const InfoCallbacks*, const char*);
	typedef Mi* (*createMachine)(const char* id, Callbacks* cb);
};
