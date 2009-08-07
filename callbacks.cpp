#include "stdafx.h"
#include "common.h"
#include "callbacks.h"
#include "cosecant_api.h"
using namespace CosecantAPI;
#include "eventlist.h"
#include "song.h"
#include "dllmachine.h"
#include "seqplay.h"
#include "application.h"

static int returnString(const QString& s, char* buf, int buf_size)
{
	QByteArray bytes = s.toUtf8();

	if (buf_size > 0)
	{
		strncpy(buf, bytes, buf_size-1);
		buf[buf_size-1] = '\0';
	}

	return static_cast<int>(bytes.size() + 1);
}

static unsigned int getHostVersion()
{
	return CosecantAPI::version;
}

static void debugPrint(const char* msg)
{
	qDebug(msg);
}

static void registerMiFactory(MiFactoryList* list,
							  const char* id, const char* desc, void* user, unsigned int userSize)
{
	MachineFactory::add(id, new DllMachine::Factory(list->m_dllpath, id, desc, user, userSize));
}

static const TimeInfo* getTimeInfo(HostMachine* mac)
{
	return &SeqPlay::get().getTimeInfo();
}

static void registerScriptFunction(HostMachine* mac, const char* name, int id)
{
	mac->addScriptFunction(name, id);
}

static cbool lockMutex(HostMachine* mac)
{
	return mac->m_mutex.tryLock(1000) ? ctrue : cfalse;
}

static cbool unlockMutex(HostMachine* mac)
{
	mac->m_mutex.unlock();
	return ctrue;
}

static cbool ScriptValue_isNull(const ScriptValue* v)
{
	return v->isNull() ?ctrue:cfalse;
}

static cbool ScriptValue_isValid(const ScriptValue* v)
{
	return v->isValid() ?ctrue:cfalse;
}

static cbool ScriptValue_isNumber(const ScriptValue* v)
{
	return v->isNumber() ?ctrue:cfalse;
}

static int ScriptValue_toInt(const ScriptValue* v)
{
	return v->toInt32();
}

static double ScriptValue_toDouble(const ScriptValue* v)
{
	return v->toNumber();
}

static cbool ScriptValue_isString(const ScriptValue* v)
{
	return v->isString() ?ctrue:cfalse;
}

static int ScriptValue_toString(const ScriptValue* v, char* buf, int bufsize)
{
	return returnString(v->toString(), buf, bufsize);
}

static cbool ScriptValue_isArray(const ScriptValue* v)
{
	return v->isArray() ?ctrue:cfalse;
}

static unsigned int ScriptValue_getArrayLength(const ScriptValue* v)
{
	return v->property("length").toInt32();
}

static const ScriptValue* ScriptValue_getArrayElement(const ScriptValue* v, unsigned int index)
{
	return new QScriptValue(v->property(index));
}

static Mi* ScriptValue_toMi(const ScriptValue* v)
{
	return NULL;
}

static MiPattern* ScriptValue_toMiPattern(const ScriptValue* v)
{
	QObject* qo = v->toQObject();
	if (qo)
	{
		if (DllMachine::Pattern* dllpat = dynamic_cast<DllMachine::Pattern*>(qo))
		{
			return dllpat->getMiPattern();
		}
	}

	return NULL;
}

static ScriptValue* ScriptValue_createNull()
{
	return new QScriptValue(QScriptValue::NullValue);
}

static ScriptValue* ScriptValue_createInvalid()
{
	return new QScriptValue(QScriptValue::UndefinedValue);
}

static ScriptValue* ScriptValue_createInt(int v)
{
	return new QScriptValue(v);
}

static ScriptValue* ScriptValue_createDouble(double v)
{
	return new QScriptValue(v);
}

static ScriptValue* ScriptValue_createString(const char* v)
{
	return new QScriptValue(v);
}

static void ScriptValue_destroy(const ScriptValue* v)
{
	delete v;
}

static ScriptValue* ScriptValue_createArray(unsigned int length)
{
	return new QScriptValue(Application::get().getScriptEngine()->newArray(length));
}

static void ScriptValue_setArrayElement(ScriptValue* arr, unsigned int index, ScriptValue* value, cbool takeOwnership)
{
	arr->setProperty(index, *value);
	if (takeOwnership) delete value;
}

///////////////////////////////////////////////////////////////////////////

static HostFunctions g_hostFuncs =
{
	getHostVersion,
	debugPrint,
	registerMiFactory,
	getTimeInfo,
	registerScriptFunction,
	lockMutex,
	unlockMutex,
	ScriptValue_isNull,
	ScriptValue_isValid,
	ScriptValue_isNumber,
	ScriptValue_toInt,
	ScriptValue_toDouble,
	ScriptValue_isString,
	ScriptValue_toString,
	ScriptValue_isArray,
	ScriptValue_getArrayLength,
	ScriptValue_getArrayElement,
	ScriptValue_toMi,
	ScriptValue_toMiPattern,
	ScriptValue_createNull,
	ScriptValue_createInvalid,
	ScriptValue_createInt,
	ScriptValue_createDouble,
	ScriptValue_createString,
	ScriptValue_destroy,
	ScriptValue_createArray,
	ScriptValue_setArrayElement,
};

HostFunctions* CosecantAPI::g_host = &g_hostFuncs;
