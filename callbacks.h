#pragma once

#include "cosecant_api.h"
using namespace CosecantAPI;

class CallbacksImpl : public Callbacks
{
public:
	virtual unsigned int getHostVersion() { return CosecantAPI::version; }

	virtual bool lockMutex(HostMachine*);
	virtual void unlockMutex(HostMachine*);

	virtual double getTicksPerFrame();

	virtual void addParamChange(PinBuffer* buf, int time, ParamValue value);
	virtual void addNoteEvent  (PinBuffer* buf, int time, NoteEvent* ev);

	virtual void doUndoable(HostMachine*, MiUndoable*);

	virtual void xmlSetAttribute_c(XmlElement*, const char* name, const char* value);
	virtual XmlElement* xmlAddChild(XmlElement*, const char* tag);

	virtual int xmlGetAttribute_c(XmlElement*, const char* name, char* value, int value_size);
	virtual int xmlGetTagName_c(XmlElement*, char* value, int value_size);
	virtual XmlElement* xmlGetFirstChild(XmlElement*);
	virtual XmlElement* xmlGetNextSibling(XmlElement*);

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
