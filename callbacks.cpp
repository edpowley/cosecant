#include "stdafx.h"
#include "common.h"
#include "callbacks.h"
#include "cosecant_api.h"
using namespace CosecantAPI;
#include "song.h"
#include "dllmachine.h"
#include "seqplay.h"
#include "application.h"

static int32_t returnString(const QString& s, char* buf, int32_t buf_size)
{
	QByteArray bytes = s.toUtf8();

	if (buf_size > 0)
	{
		strncpy(buf, bytes, buf_size-1);
		buf[buf_size-1] = '\0';
	}

	return static_cast<int32_t>(bytes.size() + 1);
}

static uint32_t getHostVersion()
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

static int32_t toUtf8(char* buf, int32_t bufsize, const wchar_t* str)
{
	return returnString(QString::fromWCharArray(str), buf, bufsize);
}

static void registerMiFactory(MiFactoryList* list,
							  const char* id, const char* desc, void* user, uint32_t userSize)
{
	MachineFactory::add(id, new DllMachine::Factory(list->m_dllpath, id, desc, user, userSize));
}

static const TimeInfo* getTimeInfo(HostMachine* mac)
{
	return &SeqPlay::get().getTimeInfo();
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

static void addParamChangeEvent(PinBuffer* buf, int32_t time, double value)
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
		const EventStream* stream;
		EventStream::const_iterator i;

		EventStreamIter(const EventStream* s) : stream(s) {}
	};
};

static EventStreamIter* createEventStreamIter(const PinBuffer* buf)
{
	if (const WorkBuffer::Events* es = dynamic_cast<const WorkBuffer::Events*>(buf->hostbuf))
		return new EventStreamIter(&es->m_data);
	else
		return NULL;
}

static EventStreamIter* EventStream_begin(const PinBuffer* buf)
{
	EventStreamIter* i = createEventStreamIter(buf);
	if (i) i->i = i->stream->begin();
	return i;
}

static EventStreamIter* EventStream_end(const PinBuffer* buf)
{
	EventStreamIter* i = createEventStreamIter(buf);
	if (i) i->i = i->stream->end();
	return i;
}

static EventStreamIter* EventStream_find(const PinBuffer* buf, int32_t key)
{
	EventStreamIter* i = createEventStreamIter(buf);
	if (i) i->i = i->stream->find(key);
	return i;
}

static EventStreamIter* EventStream_lowerBound(const PinBuffer* buf, int32_t key)
{
	EventStreamIter* i = createEventStreamIter(buf);
	if (i) i->i = i->stream->lowerBound(key);
	return i;
}

static EventStreamIter* EventStream_upperBound(const PinBuffer* buf, int32_t key)
{
	EventStreamIter* i = createEventStreamIter(buf);
	if (i) i->i = i->stream->upperBound(key);
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

static int32_t EventStreamIter_deref(EventStreamIter* iter, StreamEvent* ev, uint32_t evSize)
{
	if (!iter) return 0;
	if (ev && evSize)
		memcpy(ev, &*iter->i, min(evSize, sizeof(StreamEvent)));
	return iter->i->time;
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
};

HostFunctions* CosecantAPI::g_host = &g_hostFuncs;
