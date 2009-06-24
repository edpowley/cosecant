#include "stdafx.h"
#include "common.h"
#include "routing.h"

Routing::Routing()
{
	ctorCommon();
}

void Routing::ctorCommon()
{
	m_changeBatchCounter = 0;
	connect(
		this, SIGNAL(signalTopologyChange()),
		this, SIGNAL(signalChange())
	);
}

void Routing::addMachine(const  Ptr<Machine>& machine)
{
	machine->m_routing = this;
	m_machines.push_back(machine);
	signalTopologyChangeIfNotInBatch();
	signalAddMachine(machine);
	machine->added();
}

void Routing::removeMachine(const  Ptr<Machine>& machine)
{
	vectorEraseFirst(m_machines, machine);
	signalTopologyChangeIfNotInBatch();
	signalRemoveMachine(machine);
	machine->removed();
}

Ptr<Connection> Routing::createConnection(const Ptr<Pin> &pin1, const Ptr<Pin> &pin2)
{
	// Same direction => don't connect
	if (pin1->m_direction == pin2->m_direction)
	{
		throw CreateConnectionError(QString("Cannot connect %1 pin to another %1 pin.")
			.arg((pin1->m_direction == Pin::in) ? "input" : "output")
		);
	}

	// Directions the wrong way around => swap pins and try again
	if (pin1->m_direction == Pin::in)
		return createConnection(pin2, pin1);

	// Sanity check
	if (pin1->m_direction != Pin::out || pin2->m_direction != Pin::in)
		THROW_ERROR(CreateConnectionError, "Direction sanity check failed");

	// Check types
	if (pin1->m_type != pin2->m_type)
		throw CreateConnectionError(QString("Cannot connect pin of type '%1' to pin of type '%2'.")
			.arg(WorkBuffer::getDescription(pin1->m_type), WorkBuffer::getDescription(pin2->m_type))
		);

	// Don't create it if it already exists
	for (std::vector< Ptr<Connection> >::iterator i = pin1->m_connections.begin();
		i != pin1->m_connections.end(); ++i)
	{
		if ((*i)->getPin1() == pin1 && (*i)->getPin2() == pin2)
			throw CreateConnectionError("This connection already exists.");
	}

	// Create it
	bool feedback = existsPathBetweenMachines(pin2->m_machine, pin1->m_machine);
	Ptr<Connection> p(new Connection(pin1, pin2, feedback));
	return p;
}

void Routing::addConnection(const Ptr<Connection> &conn)
{
	conn->getPin1()->m_connections.push_back(conn);
	conn->getPin2()->m_connections.push_back(conn);
	signalTopologyChangeIfNotInBatch();
}

void Routing::removeConnection(const Ptr<Connection> &conn)
{
	vectorEraseFirst(conn->getPin1()->m_connections, conn);
	vectorEraseFirst(conn->getPin2()->m_connections, conn);
	signalTopologyChangeIfNotInBatch();
}

bool Routing::existsPathBetweenMachines(Machine* mac1, Machine* mac2, bool usefb)
{
	if (mac1 == mac2) return true;

	for (std::vector< Ptr<Pin> >::iterator pi = mac1->m_outpins.begin();
		pi != mac1->m_outpins.end(); ++pi)
	{
		for (std::vector< Ptr<Connection> >::iterator ci = (*pi)->m_connections.begin();
			ci != (*pi)->m_connections.end(); ++ci)
		{
			// skip feedback connections
			if (!usefb && (*ci)->m_feedback)
				continue;

			if (existsPathBetweenMachines((*ci)->getPin2()->m_machine, mac2, usefb))
				return true;
		}
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////////

void Routing::findAllPaths(Machine *mac1, Machine *mac2,
						   bool useFeedbackConnections,
						   findAllPaths_callback& callback)
{
	BOOST_FOREACH(const Ptr<Pin>& pin, mac1->m_outpins)
	{
		BOOST_FOREACH(const Ptr<Connection>& conn, pin->m_connections)
		{
			if (useFeedbackConnections || !conn->m_feedback)
			{
				std::vector< Ptr<Connection> > path;
				path.push_back(conn);
				findAllPaths_Helper(path, mac2, useFeedbackConnections, callback);
			}
		}
	}
}

void Routing::findAllPaths_Helper(std::vector< Ptr<Connection> >& partialPath,
								  Machine* target,
								  bool useFeedbackConnections,
								  findAllPaths_callback& callback)
{
	Machine* back = partialPath.back()->getPin2()->m_machine;
	if (back == target)
	{
		callback(partialPath);
		return;
	}

	BOOST_FOREACH(const Ptr<Pin>& pin, back->m_outpins)
	{
		BOOST_FOREACH(const Ptr<Connection>& conn, pin->m_connections)
		{
			if (useFeedbackConnections || !conn->m_feedback)
			{
				partialPath.push_back(conn);
				findAllPaths_Helper(partialPath, target, useFeedbackConnections, callback);
				partialPath.pop_back();
			}
		}
	}
}
