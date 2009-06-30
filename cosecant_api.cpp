#include "stdafx.h"
#include "cosecant_api.h"
using namespace CosecantAPI;

#define DLLEXPORT(RETURN_TYPE) extern "C" __declspec(dllexport) RETURN_TYPE

////////////////////////////////////////////////////////////////////////////

namespace MachineFunctionImpl
{
	void destroy(Mi* mi)
	{	delete mi;   }

	void work(Mi* mi, PinBuffer* inpins, PinBuffer* outpins, int first, int last)
	{	mi->work(inpins, outpins, first, last);   }

	void changeParam(Mi* mi, ParamTag tag, ParamValue value)
	{	mi->changeParam(tag, value);   }

	void* noteOn(Mi* mi, double note, double vel)
	{	return mi->noteOn(note, vel);   }

	void noteOff(Mi* mi, void* note)
	{	mi->noteOff(note);   }

	void playPattern(Mi* mi, void* track, MiPattern* pattern, double pos)
	{	mi->playPattern(track, pattern, pos);   }

	MiPattern* newPattern(Mi* mi, int length)
	{	return mi->createPattern(length);   }

	bool patLoad(MiPattern* pattern, XmlElement* el)
	{	
		try { pattern->load(el); return true;}
		catch (const LoadError&) { return false; }
	}

	void patSave(MiPattern* pattern, XmlElement* el)
	{	pattern->save(el);   }

	MiUndoable* patChangeLength(MiPattern* pattern, int newlength)
	{	return pattern->createUndoableForLengthChange(newlength);   }

	void deletePattern(MiPattern* pattern)
	{	delete pattern;   }

	MiPatternEditor* createPatternEditor(Mi* mi, WindowHandle parent, MiPattern* pattern)
	{	return mi->createPatternEditor(parent, pattern);   }

	void patedTakeFocus(MiPatternEditor* editor)
	{	editor->takeFocus();   }

	void patedKeyJazz(MiPatternEditor* editor, KeyJazz::Type type, KeyJazz::Note note)
	{	editor->keyJazz(type, note);   }

	void destroyPatternEditor(MiPatternEditor* editor)
	{	delete editor;   }

	void undoableDo(MiUndoable* undoable)
	{	(*undoable)();   }

	void undoableUndo(MiUndoable* undoable)
	{	undoable->undo();   }

	const char* undoableDescribe(MiUndoable* undoable)
	{	return undoable->describe();   }

	void undoableDestroy(MiUndoable* undoable)
	{	delete undoable;   }

	//////////////////////////////////////////////////////////////////////////

	static MachineFunctions functions = {
		CosecantAPI::version,

		destroy,
		work,
		changeParam,
		noteOn,
		noteOff,
		playPattern,

		newPattern,
		patLoad,
		patSave,
		patChangeLength,
		deletePattern,

		createPatternEditor,
		patedTakeFocus,
		patedKeyJazz,
		destroyPatternEditor,

		undoableDo,
		undoableUndo,
		undoableDescribe,
		undoableDestroy,
	};
};

////////////////////////////////////////////////////////////////////////////

class MachineFactories
{
public:
	static MachineFactories singleton;
	std::map<std::string, MiFactory*> m_fac;

private:
	MachineFactories()
	{
		CosecantAPI::populateMiFactories(m_fac);
	}
};

MachineFactories MachineFactories::singleton;

////////////////////////////////////////////////////////////////////////////

DLLEXPORT(void) getMachineIds(void (*cb)(void*, const char*), void* cbArg)
{
	for (std::map<std::string, MiFactory*>::const_iterator
		iter  = MachineFactories::singleton.m_fac.begin();
		iter != MachineFactories::singleton.m_fac.end();
		++ iter)
	{
		cb(cbArg, iter->first.c_str());
	}
}

////////////////////////////////////////////////////////////////////////////

DLLEXPORT(bool) getInfo(MachineInfo* info, const InfoCallbacks* cb, const char* id)
{
	std::map<std::string, MiFactory*>::const_iterator
		iter  = MachineFactories::singleton.m_fac.find(id);
	if (iter != MachineFactories::singleton.m_fac.end())
		return iter->second->getInfo(info, cb);
	else
		return false;
}

////////////////////////////////////////////////////////////////////////////

DLLEXPORT(Mi*) createMachine(const char* id, HostMachine* mac, Callbacks* cb)
{
	std::map<std::string, MiFactory*>::const_iterator
		iter  = MachineFactories::singleton.m_fac.find(id);
	if (iter != MachineFactories::singleton.m_fac.end())
		return iter->second->createMachine(mac,cb);
	else
		return NULL;
}

////////////////////////////////////////////////////////////////////////////

DLLEXPORT(MachineFunctions*) getFunctions()
{
	return &MachineFunctionImpl::functions;
}

