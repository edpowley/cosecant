#include "stdafx.h"
#include "common.h"
#include "machinedll.h"
#include "dll.h"
#include "callbacks.h"
#include "cosecant_api.h"
using namespace CosecantAPI;
#include "song.h"

Ptr<MachInfo> DllMachineFactory::getMachInfo()
{
	try
	{
		Dll dll(m_dllpath);

		MachineExports::getInfo f = (MachineExports::getInfo)dll.getFunc("getInfo");

		Ptr<MachInfo> info = new MachInfo(m_id);
		f(info, &InfoCallbacksImpl::g, m_id.toAscii());

		// TODO: error checking: if getFunc returns NULL or f returns false

		return info;
	}
	catch (const Dll::InitError& err)
	{
		printf("DllMachineFactory::getMachInfo error:\n%s\n", err.what());
		return NULL;
	}
}

Ptr<Machine> DllMachineFactory::createMachineImpl()
{
	return new DllMachine(m_dllpath, m_id);
}

DllMachine::DllMachine(const QString& dllpath, const QString& id)
{
	m_instance = new Instance(this, dllpath, id);
}

DllMachine::Instance::Instance(DllMachine* mac, const QString& dllpath, const QString& id)
{
	m_dll = new Dll(dllpath);

	MachineExports::createMachine c = (MachineExports::createMachine)m_dll->getFunc("createMachine");
	m_mi = c(id.toAscii(), mac, &CallbacksImpl::g);

	MachineExports::getFunctions gf = (MachineExports::getFunctions)m_dll->getFunc("getFunctions");
	m_functions = gf();
}

DllMachine::Instance::~Instance()
{
	m_functions->destroy(m_mi);
}

void DllMachine::changeParam(ParamTag tag, ParamValue value)
{
	m_instance->m_functions->changeParam(m_instance->m_mi, tag, value);
}

void DllMachine::work(PinBuffer *inpins, PinBuffer *outpins, int firstframe, int lastframe)
{
	m_instance->m_functions->work(m_instance->m_mi, inpins, outpins, firstframe, lastframe);
}

void* DllMachine::noteOn(double note, double vel)
{
	if (m_instance->m_functions->noteOn)
		return m_instance->m_functions->noteOn(m_instance->m_mi, note, vel);
	else
		return NULL;
}

void DllMachine::noteOff(void* note)
{
	if (m_instance->m_functions->noteOff)
		m_instance->m_functions->noteOff(m_instance->m_mi, note);
}

void DllMachine::playPattern(const Ptr<Sequence::Track>& track, const Ptr<Sequence::Event>& ev, double pos)
{
	Pattern* patt = NULL;
	if (ev) patt = dynamic_cast<Pattern*>(ev->m_pattern.c_ptr());

	if (m_instance->m_functions->playPattern)
		m_instance->m_functions->playPattern(m_instance->m_mi,
												track.c_ptr(),
												patt ? patt->m_ppat : NULL,
												pos);
}

////////////////////////////////////////////////////////////////////////////////////////

DllMachine::Pattern::Pattern(DllMachine* mac, double length)
: Sequence::Pattern(mac), m_instance(mac->m_instance), m_length(length)
{
	m_ppat = m_instance->m_functions->newPattern(m_instance->m_mi, (int)ceil(m_length));
	if (!m_ppat)
		THROW_ERROR(Error, "newPattern returned NULL");
}

DllMachine::Pattern::~Pattern()
{
	if (m_ppat)
		m_instance->m_functions->deletePattern(m_ppat);
}

Ptr<Sequence::Pattern> DllMachine::newPattern(double length)
{
	if (m_info->m_flags & MachineFlags::hasCustomPatterns)
		return new Pattern(this, length);
	else
		return Machine::newPattern(length);
}

/////////////////////////////////////////////////////////////////////////////////////

class PatternLengthChangeAction : public DllMachine::Command
{
public:
	PatternLengthChangeAction(const Ptr<DllMachine::Pattern>& pattern, double newlength, MiUndoable* miundoable)
		: Command(pattern->m_instance, miundoable),
		m_pattern(pattern), m_oldLength(pattern->m_length), m_newLength(newlength)
	{
	}

	virtual void redo()
	{
		m_pattern->m_length = m_newLength;
		Command::redo();
	}

	virtual void undo()
	{
		m_pattern->m_length = m_oldLength;
		Command::undo();
	}

protected:
	Ptr<DllMachine::Pattern> m_pattern;
	double m_oldLength, m_newLength;
};

QUndoCommand* DllMachine::Pattern::createUndoableForLengthChange(double newlength)
{
	return new PatternLengthChangeAction(this, newlength,
		m_instance->m_functions->patChangeLength(m_ppat, (int)ceil(newlength)));
}

///////////////////////////////////////////////////////////////////////////////////////

/*
DllMachine::DllPatternEditor::DllPatternEditor(const Ptr<DllMachine>& mac, const Ptr<DllMachine::Pattern>& pattern)
: PatternEditor(pattern), m_instance(mac->m_instance), m_pattern(pattern), m_peditor(NULL)
{
}

DllMachine::DllPatternEditor::~DllPatternEditor()
{
	m_pattern->onEditorClose(this);
}

void DllMachine::DllPatternEditor::on_realize()
{
	PatternEditor::on_realize();

	m_socket = new Gtk::Socket;
	pack_start(*m_socket);
	show_all();

	m_peditor = m_instance->m_functions->createPatternEditor(m_instance->m_mi, m_socket->get_id(), m_pattern->m_ppat);
	if (!m_peditor)
	{
		THROW_ERROR(Error, "createPatternEditor returned NULL");
	}
}

void DllMachine::DllPatternEditor::on_unrealize()
{
	m_instance->m_functions->destroyPatternEditor(m_peditor);
	m_socket = NULL;

	PatternEditor::on_unrealize();
}
*/
PatternEditor* DllMachine::createPatternEditor(const Ptr<Sequence::Pattern>& pattern)
{
	return NULL;

/*	if (m_info->m_flags & MachineFlags::hasCustomPatterns)
		return new DllPatternEditor(this, dynamic_cast<DllMachine::Pattern*>(pattern.c_ptr()));
	else
		return Machine::createPatternEditor(pattern);
*/
}
/*
void DllMachine::DllPatternEditor::takeFocus()
{
	m_instance->m_functions->patedTakeFocus(m_peditor);
}

void DllMachine::DllPatternEditor::keyJazz(KeyJazz::Type type, KeyJazz::Note note)
{
	m_instance->m_functions->patedKeyJazz(m_peditor, type, note);
}

bool DllMachine::DllPatternEditor::on_key_press_event(GdkEventKey* ev)
{
	bool ctrl  = (ev->state & GDK_CONTROL_MASK) != 0;
	bool alt   = (ev->state & GDK_MOD1_MASK) != 0;
	bool shift = (ev->state & GDK_SHIFT_MASK) != 0;

	if (!ctrl && !alt && !shift)
	{
//		switch (ev->keyval)
		{
		}
	}

	return false;
}
*/
//////////////////////////////////////////////////////////////////////////////////////

void DllMachineFactory::scan(const QString& path)
{
	QDir dir(path);
	foreach(QString fname, dir.entryList(QStringList("*.dll"), QDir::Files))
	{
		QString fpath = path + "/" + fname;
		try
		{
			Dll dll(fpath);

			MachineExports::getMachineIds f
				= (MachineExports::getMachineIds)dll.getFunc("getMachineIds");
			if (f)
			{
				struct Callback
				{
					const QString& m_fname;
					Callback(const QString& fname) : m_fname(fname) {}

					static void callback(void* vinst, const char* id)
					{
						Callback* inst = (Callback*)vinst;
						Ptr<DllMachineFactory> fac = new DllMachineFactory(inst->m_fname, id);
						MachineFactory::add(id, fac);
					}
				};

				Callback cb(fpath);
				f(Callback::callback, (void*)&cb);
			}
		}
		catch (const Dll::InitError& err)
		{
			qDebug() << "Failed to load dll " << fpath << ": " << err.msg();
		}
	}
}
