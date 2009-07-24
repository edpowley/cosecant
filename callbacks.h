#pragma once

#include "cosecant_api.h"
using namespace CosecantAPI;

class CallbacksImpl : public Callbacks
{
public:
	static int returnString(const QString& s, char* buf, int buf_size);

	CallbacksImpl(const Ptr<Machine>& mac) : m_mac(mac) {}

	virtual unsigned int getHostVersion() { return CosecantAPI::version; }
	virtual HostMachine* getHostMachine() { return m_mac.c_ptr(); }

	virtual void debugPrint(const char* msg) { qDebug(msg); }

	virtual const TimeInfo* getTimeInfo();

	virtual void addScriptFunction(const char* name, int id);

	virtual void addParamChange(PinBuffer* buf, int time, ParamValue value);
	virtual void addNoteEvent  (PinBuffer* buf, int time, NoteEvent* ev);

	virtual Script::ValuePtr scriptValueNull();
	virtual Script::ValuePtr scriptValue(bool v);
	virtual Script::ValuePtr scriptValue(int v);
	virtual Script::ValuePtr scriptValue(double v);
	virtual Script::ValuePtr scriptValue(const char* v);

protected:
	Ptr<Machine> m_mac;

	virtual bool lockMutex();
	virtual void unlockMutex();
};

class ScriptValueImpl : public Script::Value
{
public:
	ScriptValueImpl(const QScriptValue& v) : m_v(v), m_refcount(0) {}
	QScriptValue getQValue() { return m_v; }

	virtual bool isValid()		{ return m_v.isValid(); }
	virtual bool isNull()		{ return m_v.isNull(); }

	virtual bool isBool()		{ return m_v.isBool(); }
	virtual bool toBool()		{ return m_v.toBool(); }

	virtual bool isNumber()		{ return m_v.isNumber(); }
	virtual int toInt32()		{ return m_v.toInt32(); }
	virtual double toInteger()	{ return m_v.toInteger(); }
	virtual double toNumber()	{ return m_v.toNumber(); }

	virtual bool isString()		{ return m_v.isString(); }
	virtual int toString(char* buf, int bufsize)
	{ return CallbacksImpl::returnString(m_v.toString(), buf, bufsize); }

protected:
	virtual void incRef() { InterlockedIncrement(&m_refcount); }
	virtual void decRef() { if (InterlockedDecrement(&m_refcount) == 0) delete this; }

	__declspec(align(4)) long m_refcount;
	QScriptValue m_v;
};

class ScriptArgumentsImpl : public Script::Arguments
{
public:
	ScriptArgumentsImpl(QScriptContext* c) : m_c(c) {}

	// First argument is the function name
	virtual int count() { return m_c->argumentCount() - 1; }
	virtual Script::ValuePtr operator[](int i) { return new ScriptValueImpl(m_c->argument(i+1)); }

protected:
	QScriptContext* m_c;
};

namespace MachineExports
{
	using namespace CosecantAPI;

	typedef unsigned int (*getAPIVersion)();
	typedef void (*getMachineIds)(void (*)(void*, const char*), void*);
	typedef bool (*getInfo)(MachineInfo*, const InfoCallbacks*, const char*);
	typedef Mi* (*createMachine)(const char* id, Callbacks* cb);
};
