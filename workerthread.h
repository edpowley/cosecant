#pragma once

class WorkerThread
{
public:
	WorkerThread() : m_exit(false) {}
	void operator()();

	bool m_exit;
};
