#pragma once

#include "machine.h"
#include "signal.h"

class Routing : public ObjectWithUuid
{
	Q_OBJECT

	friend Machine;
	friend class Song;

public:
	Routing();
	Routing(class SongLoadContext& ctx, xmlpp::Element* el);

signals:
	// m_signalTopologyChange fires when the topology (machines, pins, connections) of the routing changes
	// m_signalChange fires when the topology changes, and on changes which do not affect the topology
	// (renaming/repositioning etc)
	// The intention being that m_signalChange fires for things which require a redraw of the routing editor, while
	// m_signalTopologyChange only fires for things which require rebuilding the work queue
	void signalChange();
	void signalTopologyChange();
	void signalAddMachine(const Ptr<Machine>& mac);
	void signalRemoveMachine(const Ptr<Machine>& mac);

public:
	size_t getNumMachines() { return m_machines.size(); }
	Ptr<Machine> getMachine(size_t index) { return m_machines[index]; }

	void addMachine(const Ptr<Machine>& machine);
	void removeMachine(const Ptr<Machine>& machine);

	ERROR_CLASS(CreateConnectionError);
	Ptr<Connection> createConnection(const Ptr<Pin>& pin1, const Ptr<Pin>& pin2);
	void addConnection(const Ptr<Connection>& conn);
	void removeConnection(const Ptr<Connection>& conn);

	bool existsPathBetweenMachines(Machine* mac1, Machine* mac2, bool useFeedbackConnections = false);

	typedef
		boost::function<
			void (
				const std::vector<
					Ptr<Connection>
				>&
			)
		>
		findAllPaths_callback;

	void findAllPaths(Machine* mac1, Machine* mac2, bool useFeedbackConnections,
		 findAllPaths_callback& callback);

	std::vector< Ptr<Machine> > m_machines;

	void save(xmlpp::Element* el);

	class ChangeBatch
	{
	public:
		ChangeBatch(Routing* routing) : m_routing(routing)
		{
			++ m_routing->m_changeBatchCounter;
		}

		~ChangeBatch()
		{
			-- m_routing->m_changeBatchCounter;
			m_routing->signalTopologyChangeIfNotInBatch();
		}

	protected:
		Routing* m_routing;
	};

protected:
	void ctorCommon();

	void onTopologyChange();
	
	int m_changeBatchCounter;

	void signalTopologyChangeIfNotInBatch()
	{
		if (m_changeBatchCounter <= 0) signalTopologyChange();
	}

	void findAllPaths_Helper(std::vector< Ptr<Connection> >& partialPath,
							 Machine* target,
							 bool useFeedbackConnections,
							 findAllPaths_callback& callback);

};

