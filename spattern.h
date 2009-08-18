#pragma once

#include "sequence.h"

class SpatternMachine;

class Spattern : public Sequence::Pattern
{
public:
	Spattern(SpatternMachine* mac, double length);

	virtual void play(Sequence::Track* track, double startpos);
	virtual void stop(Sequence::Track* track);

protected:
	SpatternMachine* m_mac;
};
