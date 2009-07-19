#pragma once

#include "cosecant_api.h"
using namespace CosecantAPI;

class CallbacksImpl : public Callbacks
{
public:
	virtual unsigned int getHostVersion() { return CosecantAPI::version; }

	virtual bool lockMutex(HostMachine*);
	virtual void unlockMutex(HostMachine*);

	virtual const TimeInfo* getTimeInfo();

	virtual void addParamChange(PinBuffer* buf, int time, ParamValue value);
	virtual void addNoteEvent  (PinBuffer* buf, int time, NoteEvent* ev);

	virtual void doUndoable(HostMachine*, MiUndoable*);

protected:
	int returnString(const QString& s, char* buf, int buf_size);
};

namespace MachineExports
{
	using namespace CosecantAPI;

	typedef unsigned int (*getAPIVersion)();
	typedef void (*getMachineIds)(void (*)(void*, const char*), void*);
	typedef bool (*getInfo)(MachineInfo*, const InfoCallbacks*, const char*);
	typedef Mi* (*createMachine)(const char* id, HostMachine* mac, Callbacks* cb);
};
