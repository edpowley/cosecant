#pragma once

#include "workunit.h"
#include "workbuffer.h"

class WorkQueue : public Object
{
protected:
	WorkQueue(const Ptr<class Routing>& routing);

public:
	static Ptr<WorkQueue> s; // singleton

	static void updateFromSongRouting();
	static void setNull();

	static boost::shared_mutex s_singletonMutex;

	boost::shared_mutex m_mutex;

	// Requires m_mutex
	WorkUnit::Base* popReady();

	std::vector< Ptr<WorkUnit::Base> > m_units;
	std::vector<WorkUnit::Base*> m_ready; // Ptr probably isn't very thread safe

	size_t m_toWork;
	bool finished() { return m_toWork == 0; }

	void reset();

	std::vector< Ptr<WorkBuffer::Base> > m_workBuffersForPreProcess;

	Ptr<Sequence::Seq> m_sequence;
	bool m_playing;
	double m_playPos, m_ticksPerFrame, m_framesPerTick;
	bool m_shouldUpdateSequenceFromScratch;

	void addWorkBuffer(const Ptr<WorkBuffer::Base>& wb);

	void dumpToDot(std::ostream& stream);
};
