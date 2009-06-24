#include "stdafx.h"
#include "common.h"
#include "sequenceevent.h"
#include "machine.h"

using namespace SequenceEvent;

void Note::work(const Ptr<Machine>& mac)
{
}

void Start::work(const Ptr<Machine>& mac)
{
	if (mac->m_info->m_flags & MachineFlags::hasCustomPatterns)
		mac->playPattern(m_track, m_ev, m_pos);
}

void Stop::work(const Ptr<Machine>& mac)
{
	if (mac->m_info->m_flags & MachineFlags::hasCustomPatterns)
		mac->playPattern(m_track, NULL, 0.0);
}
