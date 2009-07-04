#include "stdafx.h"
#include "common.h"
#include "callbacks.h"
#include "cosecant_api.h"
using namespace CosecantAPI;
#include "machinfo.h"
#include "eventlist.h"
#include "song.h"
#include "dllmachine.h"

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

bool CallbacksImpl::lockMutex(HostMachine* mac)
{
	return mac->m_mutex.timed_lock(boost::posix_time::milliseconds(1000));
}

void CallbacksImpl::unlockMutex(HostMachine* mac)
{
	mac->m_mutex.unlock();
}

double CallbacksImpl::getTicksPerFrame()
{
	return Song::get().m_sequence->m_ticksPerFrame;
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

void CallbacksImpl::doUndoable(HostMachine* mac, MiUndoable* undoable)
{
//	theUndo().push( new DllMachine::Command(mac->m_instance, undoable) );
}

void CallbacksImpl::xmlSetAttribute_c(xmlpp::Element* el, const char* name, const char* value)
{
	el->set_attribute(name, value);
}

xmlpp::Element* CallbacksImpl::xmlAddChild(xmlpp::Element* el, const char* name)
{
	return el->add_child(name);
}

int CallbacksImpl::xmlGetAttribute_c(xmlpp::Element* el, const char* name, char* value, int value_size)
{
	xmlpp::Attribute* attr = el->get_attribute(name);
	if (!attr)
		return 0;
	else
		return returnString(attr->get_value().c_str(), value, value_size);
}

int CallbacksImpl::xmlGetTagName_c(xmlpp::Element* el, char* value, int value_size)
{
	return returnString(el->get_name().c_str(), value, value_size);
}

XmlElement* CallbacksImpl::xmlGetFirstChild(xmlpp::Element* el)
{
	BOOST_FOREACH(xmlpp::Node* node, el->get_children())
	{
		xmlpp::Element* childEl = dynamic_cast<xmlpp::Element*>(node);
		if (childEl) return childEl;
	}
	return NULL;
}

XmlElement* CallbacksImpl::xmlGetNextSibling(xmlpp::Element* el)
{
	for (xmlpp::Node* sibNode = el->get_next_sibling(); sibNode; sibNode = sibNode->get_next_sibling())
	{
		xmlpp::Element* sibEl = dynamic_cast<xmlpp::Element*>(sibNode);
		if (sibEl) return sibEl;
	}
	return NULL;
}
