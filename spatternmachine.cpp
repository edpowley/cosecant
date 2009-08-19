#include "stdafx.h"
#include "common.h"
#include "spatternmachine.h"
#include "spattern.h"
#include "spatterneditor.h"

SpatternMachine::SpatternMachine()
{
}

void SpatternMachine::work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe)
{
}

void SpatternMachine::initInfo()
{
	static MachineInfo info;
	static bool initialised = false;
	if (!initialised)
	{
		info.defaultName = "Standard Pattern";
		info.typeHint = MachineTypeHint::control;
		info.flags = MachineFlags::hasCustomPatterns | MachineFlags::createSequenceTrack;

		static PinInfo pinNoteOut;
		pinNoteOut.name = "Note out";
		pinNoteOut.type = SignalType::noteTrigger;

		static const PinInfo* outPins[] = {&pinNoteOut, NULL};
		info.outPins = outPins;

		initialised = true;
	}

	m_info = &info;
}

void SpatternMachine::initImpl()
{
}

Ptr<Sequence::Pattern> SpatternMachine::createPatternImpl(double length)
{
	return new Spattern::Pattern(this, length);
}

QWidget* SpatternMachine::createPatternEditorWidget(const Ptr<Sequence::Pattern>& pattern)
{
	Spattern::Pattern* spatt = dynamic_cast<Spattern::Pattern*>(pattern.c_ptr());
	return new SpatternEditor::Editor(spatt);
}
