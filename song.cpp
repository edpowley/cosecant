#include "stdafx.h"
#include "common.h"
#include "song.h"
#include "workqueue.h"

Song* Song::s_singleton = NULL;

void Song::initSingleton()
{
	s_singleton = new Song();
}

Song::Song()
{
	m_routing = new Routing();
	connect(
		m_routing, SIGNAL(signalTopologyChange()),
		this, SLOT(updateWorkQueue())
	);

	m_sequence = new Seq::Sequence();
}

void Song::clear()
{
	m_undo.clear();
	m_routing = NULL;
	WorkQueue::setNull();
}

void Song::updateWorkQueue()
{
	WorkQueue::updateFromSongRouting();
}
