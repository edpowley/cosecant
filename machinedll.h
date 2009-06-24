#pragma once

#include "machine.h"
#include "dll.h"
#include "callbacks.h"
#include "sequence.h"
//#include "notebookwindow.h"
//#include "patterneditor.h"

class DllMachineFactory : public MachineFactory
{
public:
	DllMachineFactory(const bpath& dllpath, const QString& id)
		: m_dllpath(dllpath), m_id(id) {}

	virtual Ptr<MachInfo> getMachInfo();

	static void scan(const bpath& path);

protected:
	QString m_id;
	bpath m_dllpath;

	virtual Ptr<Machine> createMachineImpl();
};

class DllMachine : public Machine
{
public:
	DllMachine(const bpath& dllpath, const QString& id);

	virtual void changeParam(ParamTag tag, ParamValue value);
	virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);
	virtual void* noteOn(double note, double vel);
	virtual void noteOff(void* note);
	virtual void playPattern(const Ptr<Sequence::Track>& track, const Ptr<Sequence::Event>& ev, double pos);

	virtual PatternEditor* createPatternEditor(const Ptr<Sequence::Pattern>& pattern);

	class Instance : public Object
	{
	public:
		Instance(DllMachine* mac, const bpath& dllpath, const QString& id);
		virtual ~Instance();

		Mi* m_mi;
		MachineFunctions* m_functions;

	protected:
		Ptr<Dll> m_dll;
	};

	class Action : public Undoable
	{
	public:
		Action(Instance* instance, MiUndoable* p) : m_instance(instance), m_p(p)
		{
		}

		virtual ~Action()
		{
			m_instance->m_functions->undoableDestroy(m_p);
		}

		virtual bool operator()()
		{
			return m_instance->m_functions->undoableDo(m_p);
		}

		virtual bool undo()
		{
			return m_instance->m_functions->undoableUndo(m_p);
		}

		virtual QString describe()
		{
			return m_instance->m_functions->undoableDescribe(m_p);
		}

	protected:
		Ptr<Instance> m_instance;
		MiUndoable* m_p;
	};

	class Pattern : public Sequence::Pattern
	{
		friend class PatternLengthChangeAction;
	public:
		Pattern(DllMachine* mac, double length);
		virtual ~Pattern();

		virtual double getLength() { return m_length; }
		virtual void load(SongLoadContext& ctx, xmlpp::Element* el);
		virtual void save(xmlpp::Element* el) {}

		virtual Ptr<Undoable> createUndoableForLengthChange(double newlength);

		CosecantAPI::MiPattern* m_ppat;

	protected:
		Ptr<Instance> m_instance;
		double m_length;
	};

/*	class DllPatternEditor : public PatternEditor
	{
	public:
		DllPatternEditor(const Ptr<DllMachine>& mac, const Ptr<Pattern>& pattern);
		virtual ~DllPatternEditor();

		virtual void takeFocus();

		virtual void keyJazz(KeyJazz::Type type, KeyJazz::Note note);

	protected:
		Ptr<Instance> m_instance;
		Ptr<Pattern> m_pattern;
		Gtk::Socket* m_socket;
		MiPatternEditor* m_peditor;

		virtual void on_realize();
		virtual void on_unrealize();

		virtual bool on_key_press_event(GdkEventKey* ev);
	};
*/
	Ptr<Instance> m_instance;

protected:
	virtual Ptr<Sequence::Pattern> newPattern(double length);
};
