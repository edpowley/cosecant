#pragma once

#include "builtinmachines.h"

class SpatternMachine : public BuiltinMachine
{
public:
	SpatternMachine();

	virtual void changeParam(ParamTag tag, double value) {}
	virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);

	virtual QWidget* createPatternEditorWidget(const Ptr<Sequence::Pattern>& pattern);

protected:
	virtual void initInfo();
	virtual void initImpl();

	virtual Ptr<Sequence::Pattern> createPatternImpl(double length);
};

