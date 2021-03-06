#pragma once

#include "workunit.h"
#include "workbuffer.h"

class WorkQueue
{
protected:
	WorkQueue(const Ptr<class Routing>& routing);

public:
	static WorkQueue* s; // singleton
	static QReadWriteLock s_mutex;

	static void updateFromSongRouting();
	static void setNull();

	QMutex m_mutex;

	// Requires m_mutex
	WorkUnit::Base* popReady();

	std::vector< Ptr<WorkUnit::Base> > m_units;
	std::vector<WorkUnit::Base*> m_ready; // Ptr probably isn't very thread safe

	size_t m_toWork;
	bool finished() { return m_toWork == 0; }

	void reset();

	std::vector< Ptr<WorkBuffer::Base> > m_workBuffersForPreProcess;

	void addWorkBuffer(const Ptr<WorkBuffer::Base>& wb);

	void dumpToDot(std::ostream& stream);
};
