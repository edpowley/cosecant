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

static void pushStatus(const char* msg)
{
	Application::get().pushStatusMsg(decodeApiString(msg));
}

static void popStatus()
{
	Application::get().popStatusMsg();
}

static int toUtf8(char* buf, int bufsize, const wchar_t* str)
{
	return returnString(QString::fromWCharArray(str), buf, bufsize);
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

static void addParamChangeEvent(PinBuffer* buf, int time, double value)
{
	WorkBuffer::ParamControl* p = dynamic_cast<WorkBuffer::ParamControl*>(buf->hostbuf);
	if (p) p->m_data.insert(std::make_pair(time, value));
}

/////////////////////////////////////////////////////////////////////////////////

namespace CosecantAPI
{
	class EventStreamIter
	{
	public:
		const QMultiMap<int, StreamEvent>* map;
		QMultiMap<int, StreamEvent>::const_iterator i;

		EventStreamIter(const QMultiMap<int, StreamEvent>* map_) : map(map_) {}
	};
};

static EventStreamIter* createEventStreamIter(const PinBuffer* buf)
{
	if (const WorkBuffer::EventStream* es = dynamic_cast<const WorkBuffer::EventStream*>(buf->hostbuf))
		return new EventStreamIter(&es->m_data);
	else
		return NULL;
}

static EventStreamIter* EventStream_begin(const PinBuffer* buf)
{
	EventStreamIter* i = createEventStreamIter(buf);
	if (i) i->i = i->map->begin();
	return i;
}

static EventStreamIter* EventStream_end(const PinBuffer* buf)
{
	EventStreamIter* i = createEventStreamIter(buf);
	if (i) i->i = i->map->end();
	return i;
}

static EventStreamIter* EventStream_find(const PinBuffer* buf, int key)
{
	EventStreamIter* i = createEventStreamIter(buf);
	if (i) i->i = i->map->find(key);
	return i;
}

static EventStreamIter* EventStream_lowerBound(const PinBuffer* buf, int key)
{
	EventStreamIter* i = createEventStreamIter(buf);
	if (i) i->i = i->map->lowerBound(key);
	return i;
}

static EventStreamIter* EventStream_upperBound(const PinBuffer* buf, int key)
{
	EventStreamIter* i = createEventStreamIter(buf);
	if (i) i->i = i->map->upperBound(key);
	return i;
}

static EventStreamIter* EventStreamIter_copy(EventStreamIter* iter)
{
	if (!iter) return NULL;
	return new EventStreamIter(*iter);
}

static void EventStreamIter_destroy(EventStreamIter* iter)
{
	if (iter) delete iter;
}

static void EventStreamIter_inc(EventStreamIter* iter)
{
	if (iter)
		++ iter->i;
}

static void EventStreamIter_dec(EventStreamIter* iter)
{
	if (iter)
		-- iter->i;
}

static int EventStreamIter_deref(EventStreamIter* iter, StreamEvent* ev, unsigned int evSize)
{
	if (!iter) return -1;
	if (ev && evSize)
		memcpy(ev, &iter->i.value(), min(evSize, sizeof(StreamEvent)));
	return iter->i.key();
}

static cbool EventStreamIter_equal(EventStreamIter* a, EventStreamIter* b)
{
	return (a && b && a->i == b->i) ? ctrue : cfalse;
}

static void iteratePaths(const char* id, const char* name,
						 void (*callback)(void* user, const PathChar* path), void* user)
{
	Ptr<PrefsDirList> dirs = PrefsFile::get()->getDirList(id, decodeApiString(name));
	foreach(const QString& dir, dirs->getDirs())
	{
#		ifdef COSECANT_PATHS_ARE_WIDE_STRINGS
		{
			std::wstring pathstr = QDir::toNativeSeparators(dir).toStdWString();
			callback(user, pathstr.c_str());
		}
#		else
		{
			QByteArray pathstr = QDir::toNativeSeparators(dir).toUtf8();
			callback(user, pathstr.constData());
		}
#		endif
	}
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
	pushStatus,
	popStatus,
	toUtf8,
	registerMiFactory,
	getTimeInfo,
	registerScriptFunction,
	lockMutex,
	unlockMutex,
	addParamChangeEvent,
	EventStream_begin,
	EventStream_end,
	EventStream_find,
	EventStream_lowerBound,
	EventStream_upperBound,
	EventStreamIter_copy,
	EventStreamIter_destroy,
	EventStreamIter_inc,
	EventStreamIter_dec,
	EventStreamIter_deref,
	EventStreamIter_equal,
	iteratePaths,
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
