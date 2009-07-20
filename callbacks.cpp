#include "stdafx.h"
#include "common.h"
#include "callbacks.h"
#include "cosecant_api.h"
using namespace CosecantAPI;
#include "machinfo.h"
#include "eventlist.h"
#include "song.h"
#include "dllmachine.h"
#include "seqplay.h"

int CallbacksImpl::returnString(const QString& s, char* buf, int buf_size)
{
	QByteArray bytes = s.toUtf8();

	if (buf_size > 0)
	{
		// Yes VC++, I know, strncpy is unsafe and strncpy_s is *sooo* much safer
#		ifdef _MSC_VER
#			pragma warning(push)
#			pragma warning(disable:4996)
#		endif
		strncpy(buf, bytes, buf_size-1);
#		ifdef _MSC_VER
#			pragma warning(pop)
#		endif

		buf[buf_size-1] = '\0';
	}

	return static_cast<int>(bytes.size() + 1);
}

/////////////////////////////////////////////////////////////////////////

bool CallbacksImpl::lockMutex()
{
	return m_mac->m_mutex.tryLock(1000);
}

void CallbacksImpl::unlockMutex()
{
	m_mac->m_mutex.unlock();
}

const TimeInfo* CallbacksImpl::getTimeInfo()
{
	return &SeqPlay::get().getTimeInfo();
}

void CallbacksImpl::addParamChange(PinBuffer* buf, int time, ParamValue value)
{
	WorkBuffer::ParamControl* pc = dynamic_cast<WorkBuffer::ParamControl*>(buf->workbuffer);
	if (pc)
	{
		pc->m_data[time] = value;
	}
}

void CallbacksImpl::addNoteEvent(PinBuffer* buf, int time, NoteEvent* ev)
{
	WorkBuffer::SequenceEvents* nt = dynamic_cast<WorkBuffer::SequenceEvents*>(buf->workbuffer);
	if (nt)
	{
		nt->m_data.insert(std::make_pair(time, new SequenceEvent::Note(*ev)));
	}
}
