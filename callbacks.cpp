#include "stdafx.h"
#include "common.h"
#include "callbacks.h"
#include "cosecant_api.h"
using namespace CosecantAPI;
#include "machinfo.h"
#include "eventlist.h"
#include "song.h"
#include "machinedll.h"

namespace InfoCallbacksImpl
{
	void setName(MachineInfo* info, const char* name)
	{
		info->setName(name);
	}

	void setTypeHint(MachineInfo* info, MachineTypeHint::mt typehint)
	{
		info->setTypeHint(typehint);
	}

	void addInPin(MachineInfo* info, const char* name, SignalType::st type)
	{
		info->addInPin(new PinInfo(name, type));
	}

	void addOutPin(MachineInfo* info, const char* name, SignalType::st type)
	{
		info->addOutPin(new PinInfo(name, type));
	}

	void setParams(MachineInfo* info, ParamGroup* params)
	{
		info->setParams(params);
	}
	
	ParamGroup* createParamGroup(const char* name, ParamTag tag)
	{
		return new ParamInfo::Group(name, tag);
	}

	void addRealParam(ParamGroup* group, const char* name, ParamTag tag,
						double mini, double maxi, double def, unsigned long flags)
	{
		group->add(new ParamInfo::Real(name, tag, mini, maxi, def, flags));
	}

	void addTimeParam(ParamGroup* group, const char* name, ParamTag tag,
						TimeUnit::unit internalUnit, double mini, double maxi, double def,
						unsigned int guiUnits, TimeUnit::unit guiDefaultUnit)
	{
		group->add(new ParamInfo::Time(name, tag, internalUnit, mini, maxi, def, guiUnits, guiDefaultUnit));
	}

	void addEnumParam(ParamGroup* group, const char* name, ParamTag tag,
						const char* items, unsigned int def)
	{
		QStringList qsl = QString(items).split("\n", QString::SkipEmptyParts);
		std::vector<QString> itemvec(qsl.begin(), qsl.end());
		group->add(new ParamInfo::Enum(name, tag, itemvec, def));
	}

	void addSubGroup(ParamGroup* group, ParamGroup* sub)
	{
		group->add(sub);
	}

	void addFlags(MachineInfo* info, unsigned int flags)
	{
		info->addFlags(flags);
	}

	CosecantAPI::InfoCallbacks g = {
		CosecantAPI::version,
		setName,
		setTypeHint,
		addInPin,
		addOutPin,
		setParams,
		createParamGroup,
		addRealParam,
		addTimeParam,
		addEnumParam,
		addSubGroup,
		addFlags,
	};
};

namespace CallbacksImpl
{
	int returnString(const QString& s, char* buf, int buf_size)
	{
		QByteArray bytes = s.toUtf8();

		if (buf_size > 0)
		{
			// Yes VC++, I know, strncpy is unsafe and strncpy_s is *sooo* much safer
#			ifdef _MSC_VER
#				pragma warning(push)
#				pragma warning(disable:4996)
#			endif
			strncpy(buf, bytes, buf_size-1);
#			ifdef _MSC_VER
#				pragma warning(pop)
#			endif

			buf[buf_size-1] = '\0';
		}

		return static_cast<int>(bytes.size() + 1);
	}

	/////////////////////////////////////////////////////////////////////////

	bool lockMutex(HostMachine* mac)
	{
		return mac->m_mutex.timed_lock(boost::posix_time::milliseconds(1000));
	}

	void unlockMutex(HostMachine* mac)
	{
		mac->m_mutex.unlock();
	}

	double getTicksPerFrame()
	{
		return Song::get().m_sequence->m_ticksPerFrame;
	}

	void addParamChange(PinBuffer* buf, int time, ParamValue value)
	{
		WorkBuffer::ParamControl* pc = dynamic_cast<WorkBuffer::ParamControl*>(buf->workbuffer);
		if (pc)
		{
			pc->m_data[time] = value;
		}
	}

	void addNoteEvent(PinBuffer* buf, int time, NoteEvent* ev)
	{
		WorkBuffer::SequenceEvents* nt = dynamic_cast<WorkBuffer::SequenceEvents*>(buf->workbuffer);
		if (nt)
		{
			nt->m_data.insert(std::make_pair(time, new SequenceEvent::Note(*ev)));
		}
	}

	void doUndoable(HostMachine* mac, MiUndoable* undoable)
	{
		Song::get().m_undo.push( new DllMachine::Command(mac->m_instance, undoable) );
	}

	void xmlSetAttribute(xmlpp::Element* el, const char* name, const char* value)
	{
		el->set_attribute(name, value);
	}

	xmlpp::Element* xmlAddChild(xmlpp::Element* el, const char* name)
	{
		return el->add_child(name);
	}

	int xmlGetAttribute(xmlpp::Element* el, const char* name, char* value, int value_size)
	{
		xmlpp::Attribute* attr = el->get_attribute(name);
		if (!attr)
			return 0;
		else
			return returnString(attr->get_value().c_str(), value, value_size);
	}

	int xmlGetTagName(xmlpp::Element* el, char* value, int value_size)
	{
		return returnString(el->get_name().c_str(), value, value_size);
	}

	XmlElement* xmlGetFirstChild(xmlpp::Element* el)
	{
		BOOST_FOREACH(xmlpp::Node* node, el->get_children())
		{
			xmlpp::Element* childEl = dynamic_cast<xmlpp::Element*>(node);
			if (childEl) return childEl;
		}
		return NULL;
	}

	XmlElement* xmlGetNextSibling(xmlpp::Element* el)
	{
		for (xmlpp::Node* sibNode = el->get_next_sibling(); sibNode; sibNode = sibNode->get_next_sibling())
		{
			xmlpp::Element* sibEl = dynamic_cast<xmlpp::Element*>(sibNode);
			if (sibEl) return sibEl;
		}
		return NULL;
	}

	CosecantAPI::Callbacks g = {
		CosecantAPI::version,
		lockMutex,
		unlockMutex,
		getTicksPerFrame,
		addParamChange,
		addNoteEvent,
		doUndoable,
		xmlSetAttribute,
		xmlAddChild,
		xmlGetAttribute,
		xmlGetTagName,
		xmlGetFirstChild,
		xmlGetNextSibling,
	};
};
