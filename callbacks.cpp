#include "stdafx.h"
#include "common.h"
#include "callbacks.h"
#include "cosecant_api.h"
using namespace CosecantAPI;
#include "eventlist.h"
#include "song.h"
#include "dllmachine.h"
#include "seqplay.h"

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


///////////////////////////////////////////////////////////////////////////

QScriptValue variantToScriptValue(const Variant& var)
{
	switch (var.type)
	{
	case Variant::tInt:
		return var.dInt;
	case Variant::tDouble:
		return var.dDouble;
	case Variant::tString:
		return var.dString;
	case Variant::tNull:
	default:
		return QScriptValue::NullValue;
	}
}

/////////////////////////////////////////////////////////////////////////

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
	ScriptValue_toMi,
	ScriptValue_toMiPattern,
};

HostFunctions* CosecantAPI::g_host = &g_hostFuncs;
