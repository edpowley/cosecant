#include "stdafx.h"
#include "common.h"
#include "spattern.h"
#include "spatternmachine.h"

Spattern::Spattern(SpatternMachine* mac, double length)
: m_mac(mac), Sequence::Pattern(mac, length)
{
}

void Spattern::play(Sequence::Track* track, double startpos)
{
}

void Spattern::stop(Sequence::Track* track)
{
}
