#include "stdafx.h"
#include "common.h"
#include "workqueue.h"
#include "routing.h"
#include "song.h"

WorkQueue* WorkQueue::s = NULL;

void WorkQueue::reset()
{
	m_ready.clear();
	m_toWork = m_units.size();

	BOOST_FOREACH(const Ptr<WorkBuffer::Base> wb, m_workBuffersForPreProcess)
	{
		wb->preProcess();
	}

	for (std::vector< Ptr<WorkUnit::Base> >::iterator i = m_units.begin();
		i != m_units.end(); ++i)
	{
		(*i)->resetDeps();
		if ((*i)->ready())
			m_ready.push_back(*i);
	}
}

WorkUnit::Base* WorkQueue::popReady()
{
	if (!m_ready.empty())
	{
		WorkUnit::Base* ret = m_ready.back();
		m_ready.pop_back();
		return ret;
	}
	else
		return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////

namespace PseudoMachine
{
	class Base : public Machine
	{
	public:
		virtual Mi* createMi(Callbacks* cb) { THROW_ERROR(Error, "PseudoMachine should never be createMi'd"); }
	};

	class FanIn : public Base
	{
	public:
		SignalType::st m_type;
		size_t m_n;

		FanIn(size_t n, SignalType::st type) : m_type(type), m_n(n)
		{
			for (size_t i=0; i<n; i++)
				m_inpins.push_back(new Pin(this, Pin::in, type));
			m_outpins.push_back(new Pin(this, Pin::out, type));
		}
	};

	class FeedbackRead : public Base
	{
	public:
		FeedbackRead(SignalType::st type, const Ptr<DelayLine::Base>& line)
			: m_line(line)
		{
			m_outpins.push_back(new Pin(this, Pin::out, type));
		}

		Ptr<DelayLine::Base> m_line;
	};

	class FeedbackWrite : public Base
	{
	public:
		FeedbackWrite(SignalType::st type, const Ptr<DelayLine::Base>& line)
			: m_line(line)
		{
			m_inpins.push_back(new Pin(this, Pin::in, type));
		}

		Ptr<DelayLine::Base> m_line;
	};

	struct FeedbackPair
	{
		Ptr<FeedbackRead> read;
		Ptr<FeedbackWrite> write;

		FeedbackPair(SignalType::st type, const Ptr<DelayLine::Base>& line)
		{
			read = new FeedbackRead(type, line);
			write = new FeedbackWrite(type, line);
		}
	};

	class Compound : public Base
	{
	public:
		std::vector< Ptr<Machine> > m_machines;

		class Pin : public ::Pin
		{
		public:
			Compound* m_mac;
			Ptr<::Pin> m_pin;
			Pin(Compound* mac, const Ptr<::Pin> pin)
				: ::Pin(mac, pin->m_direction, pin->m_type), m_mac(mac), m_pin(pin)
			{
				m_buffer = pin->m_buffer;
			}
		};
	};
};

//////////////////////////////////////////////////////////////////////////////////

struct CtorHelper
{
	const Ptr<Routing>& routing;
	WorkQueue* wq;
	CtorHelper(WorkQueue* q, const Ptr<Routing>& r) : wq(q), routing(r) {}

	std::vector< Ptr<Machine> > machines;
	std::map< Ptr<Pin>, Ptr<Pin> > edges;
	// edges[input_pin] = output_pin, or edges[pin2] = pin1
	typedef std::pair< Ptr<Pin>, Ptr<Pin> > pinpair;
	std::vector<PseudoMachine::FeedbackPair> feedbackPairs;

	std::map< Ptr<Pin>, Ptr<WorkBuffer::Base> > pinbuffers;
	std::map<SignalType::st, Ptr<WorkBuffer::Base> > silentbuffer;

	std::map< Ptr<Machine>, Ptr<WorkUnit::Base> > workunits;

	typedef std::set< Ptr<Machine> > MachineSet;
	typedef std::vector<MachineSet> MachineSetVec;
	MachineSetVec feedbackComponents;

	void addConnection(const Ptr<Connection>& conn, const Ptr<Pin>& pin2)
	{
		if (conn->m_feedback)
		{
			/*
				pin1 -->-- pin2
			becomes
				pin1 -->-- writer ; reader -->-- pin2
			*/

			if (!conn->m_feedbackDelayLine)
			{
				conn->m_feedbackDelayLine = WorkBuffer::s_factories[pin2->m_type]
												->createDelayLine(16);
			}

			PseudoMachine::FeedbackPair fbp(pin2->m_type, conn->m_feedbackDelayLine);
			feedbackPairs.push_back(fbp);

			machines.push_back(fbp.write);
			edges[fbp.write->m_inpins[0]] = conn->getPin1();

			machines.push_back(fbp.read);
			edges[pin2] = fbp.read->m_outpins[0];
		}
		else
		{
			edges[pin2] = conn->getPin1();
		}
	}

	void initEdgesAndFanins()
	{
		BOOST_FOREACH(const Ptr<Machine>& mac, routing->m_machines)
		{
			BOOST_FOREACH(const Ptr<Pin>& pin, mac->m_inpins)
			{
				size_t indegree = pin->m_connections.size();
				if (indegree == 0)
				{
					edges[pin] = NULL;
				}
				else if (indegree == 1)
				{
					addConnection(pin->m_connections[0], pin);
				}
				else // indegree > 1
				{
					Ptr<PseudoMachine::FanIn> fanin = new PseudoMachine::FanIn(indegree, pin->m_type);
					machines.push_back(fanin);
					edges[pin] = fanin->m_outpins[0];
					for (size_t i=0; i<indegree; i++)
					{
						addConnection(pin->m_connections[i], fanin->m_inpins[i]);
					}
				}
			}
		}
	}

	void allocOutputBuffers()
	{
		// Allocate buffers for output pins that don't already have them,
		// and call addWorkBuffer for all output pin buffers
		BOOST_FOREACH(const Ptr<Machine>& mac, machines)
		{
			BOOST_FOREACH(const Ptr<Pin>& pin, mac->m_outpins)
			{
				if (!pin->m_buffer)
					pin->m_buffer = WorkBuffer::create(pin->m_type);

				wq->addWorkBuffer(pin->m_buffer);
			}
		}
	}

	void assignInputBuffers()
	{
		// Assign buffers to input pins
		// This is easy: pins with in-degree 1 are assigned the same buffer as the connected pin,
		// pins with in-degree 0 are assigned a dummy silent buffer. Pins with in-degree > 1 don't
		// exist, because we added fanins instead.

		BOOST_FOREACH(pinpair edge, edges)
		{
			if (edge.second)
			{
				pinbuffers[edge.first] = edge.second->m_buffer;
			}
			else
			{
				SignalType::st type = edge.first->m_type;
				if (!silentbuffer[type])
				{
					silentbuffer[type] = WorkBuffer::create(type);
					silentbuffer[type]->clearAll();
					wq->addWorkBuffer(silentbuffer[type]);
				}
				pinbuffers[edge.first] = silentbuffer[type];
			}
		}
	}

	void findMachinesOnReversePaths(std::vector<Machine*>& partialPath, Machine* target, MachineSet& macs)
	{
		Machine* current = partialPath.back();
		if (current == target)
		{
			macs.insert(partialPath.begin(), partialPath.end());
		}
		else
		{
			BOOST_FOREACH(const Ptr<Pin>& pin, current->m_inpins)
			{
				Ptr<Pin> pin2 = edges[pin];
				if (pin2)
				{
					partialPath.push_back(pin2->m_machine);
					findMachinesOnReversePaths(partialPath, target, macs);
					partialPath.pop_back();
				}
			}
		}
	}

	void findFeedbackComponents()
	{
		BOOST_FOREACH(const PseudoMachine::FeedbackPair& fbp, feedbackPairs)
		{
			// FB connection goes pin1 -->-- writer ; reader -->-- pin2
			// want paths from reader to writer

			MachineSet macs;
			macs.insert(fbp.read);
			macs.insert(fbp.write);
			std::vector<Machine*> path;
			path.push_back(fbp.write);

			findMachinesOnReversePaths(path, fbp.read, macs);

			feedbackComponents.push_back(macs);
		}
	}

	void uniteOverlappingFeedbackComponents()
	{
		MachineSetVec fbc2;

		BOOST_FOREACH(const MachineSet& a, feedbackComponents)
		{
			bool found = false;
			BOOST_FOREACH(MachineSet& b, fbc2)
			{
				if (containersIntersect(a,b))
				{
					b.insert(a.begin(), a.end());
					found = true;
					break;
				}
			}

			if (!found)
				fbc2.push_back(a);
		}

		std::swap(feedbackComponents, fbc2);
	}

	void printFeedbackComponents()
	{
		qDebug() << "feedbackComponents = [";
		BOOST_FOREACH(const MachineSet& ms, feedbackComponents)
		{
			qDebug() << "\t{";
			BOOST_FOREACH(const Ptr<Machine>& m, ms)
			{
				qDebug() << "\t\t" << m->getName() << " " << m->m_objectUuid.str();
			}
			qDebug() << "\t}";
		}
		qDebug() << "]";
	}

	void createCompoundMachine_Helper(const Ptr<PseudoMachine::Compound>& compound, 
									  std::set<Machine*>& macs, Machine* mac)
	{
		macs.erase(mac);

		BOOST_FOREACH(const Ptr<Pin>& pin2, mac->m_inpins)
		{
			Ptr<Pin> pin1 = edges[pin2];
			if (!pin1) continue;

			if (containerContains(macs, pin1->m_machine))
			{
				// Internal connection to a machine we haven't worked yet
				createCompoundMachine_Helper(compound, macs, pin1->m_machine);
			}
			else if (containerContains(compound->m_machines, Ptr<Machine>(pin1->m_machine)))
			{
				// Internal connection to a machine already worked
				// do nothing
			}
			else
			{
				// External connection
				Ptr<PseudoMachine::Compound::Pin> cpin = new PseudoMachine::Compound::Pin(compound, pin2);
				compound->m_inpins.push_back(cpin);
				edges.erase(pin2);
				edges[cpin] = pin1;
				pinbuffers[cpin] = pinbuffers[pin2];
			}
		}

		compound->m_machines.push_back(mac);
	}

	template<typename Iter> Ptr<PseudoMachine::Compound> createCompoundMachine(Iter macBegin, Iter macEnd)
	{
		Ptr<PseudoMachine::Compound> compound = new PseudoMachine::Compound;

		{
			std::set<Machine*> macs(macBegin, macEnd);
			while (!macs.empty())
				createCompoundMachine_Helper(compound, macs, *macs.begin());
		}

		{
			const std::set<Machine*> macs(macBegin, macEnd);

			// Find output pins
			for (std::map< Ptr<Pin>, Ptr<Pin> >::iterator iter = edges.begin(); iter != edges.end(); ++iter)
			{
				// iter->second -->-- iter->first
				if (iter->second
					&&  containerContains(macs, iter->second->m_machine)
					&& !containerContains(macs, iter->first ->m_machine))
				{
					Ptr<PseudoMachine::Compound::Pin> cpin = new PseudoMachine::Compound::Pin(compound, iter->second);
					compound->m_outpins.push_back(cpin);
					iter->second = cpin;
					pinbuffers[cpin] = pinbuffers[iter->second];
				}
			}

			// Remove internal edges
			for (std::map< Ptr<Pin>, Ptr<Pin> >::iterator iter = edges.begin(); iter != edges.end(); )
			{
				if (iter->second
					&& containerContains(macs, iter->second->m_machine)
					&& containerContains(macs, iter->first ->m_machine))
				{
					iter = edges.erase(iter);
				}
				else
				{
					++iter;
				}
			}
		}

		// Erase machines and replace with new one
		for (Iter iter = macBegin; iter != macEnd; ++iter)
		{
			vectorEraseFirst(machines, *iter);
		}

		machines.push_back(compound);

		return compound;
	}

	void collapseFeedbackComponent(const MachineSet& macs)
	{
		Ptr<PseudoMachine::Compound> compound = createCompoundMachine(macs.begin(), macs.end());
	}

	Ptr<WorkUnit::Base> createWorkUnit(Machine* mac)
	{
		if (PseudoMachine::Compound* compound = dynamic_cast<PseudoMachine::Compound*>(mac))
		{
			Ptr<WorkUnit::Chain> wu = new WorkUnit::Chain(wq);
			BOOST_FOREACH(const Ptr<Machine>& mac2, compound->m_machines)
			{
				wu->add(createWorkUnit(mac2));
			}

			return wu;
		}
		else if (PseudoMachine::FanIn* fanin = dynamic_cast<PseudoMachine::FanIn*>(mac))
		{
			Ptr<WorkUnit::FanIn> wu = new WorkUnit::FanIn(wq, fanin->m_outpins[0]->m_buffer);
			BOOST_FOREACH(const Ptr<Pin>& pin, fanin->m_inpins)
			{
				wu->addInBuf(pinbuffers[pin]);
			}

			return wu;
		}
		else if (PseudoMachine::FeedbackRead* fbread = dynamic_cast<PseudoMachine::FeedbackRead*>(mac))
		{
			Ptr<WorkUnit::FeedbackRead> wu
				= new WorkUnit::FeedbackRead(wq, fbread->m_line, fbread->m_outpins[0]->m_buffer);
			return wu;
		}
		else if (PseudoMachine::FeedbackWrite* fbwrite = dynamic_cast<PseudoMachine::FeedbackWrite*>(mac))
		{
			Ptr<WorkUnit::FeedbackWrite> wu
				= new WorkUnit::FeedbackWrite(wq, fbwrite->m_line, pinbuffers[fbwrite->m_inpins[0]]);
			return wu;
		}
		else
		{
			Ptr<WorkUnit::WorkMachine> wu = new WorkUnit::WorkMachine(wq, mac);

			BOOST_FOREACH(const Ptr<Pin>& pin, mac->m_inpins)
			{
				wu->m_inWorkBuffer.push_back(pinbuffers[pin]);
			}
			BOOST_FOREACH(const Ptr<Pin>& pin, mac->m_outpins)
			{
				wu->m_outWorkBuffer.push_back(pin->m_buffer);
			}
			wu->updatePinBuffers();

			return wu;
		}
	}

	void createWorkUnits()
	{
		BOOST_FOREACH(const Ptr<Machine>& mac, machines)
		{
			Ptr<WorkUnit::Base> wu = createWorkUnit(mac);

			workunits[mac] = wu;
			wq->m_units.push_back(wu);
		}
	}

	void setupDependencies()
	{
		BOOST_FOREACH(pinpair edge, edges)
		{
			if (edge.second)
			{
				// edges[input_pin] = output_pin
				Ptr<WorkUnit::Base> ui = workunits[edge.first ->m_machine];
				Ptr<WorkUnit::Base> uo = workunits[edge.second->m_machine];
				uo->addDependent(ui);
			}
		}
	}
};

WorkQueue::WorkQueue(const Ptr<Routing>& routing)
{
	if (!routing) return;

	// Add an artificial delay for debugging race conditions etc
#if 0
	boost::xtime sleepytime;
	boost::xtime_get(&sleepytime, boost::TIME_UTC);
	sleepytime.sec += 2;
	boost::this_thread::sleep(sleepytime);
#endif

	CtorHelper ch(this, routing);
	ch.machines = routing->m_machines;
	
	ch.initEdgesAndFanins();

	ch.allocOutputBuffers();
	ch.assignInputBuffers();

	ch.findFeedbackComponents();
	ch.uniteOverlappingFeedbackComponents();
	BOOST_FOREACH(const CtorHelper::MachineSet& macs, ch.feedbackComponents)
		ch.collapseFeedbackComponent(macs);

	ch.createWorkUnits();
	ch.setupDependencies();
}

/////////////////////////////////////////////////////////////////////////////////

void WorkQueue::addWorkBuffer(const Ptr<WorkBuffer::Base>& wb)
{
	if (wb->getFlags() & WorkBuffer::needsPreProcess)
		m_workBuffersForPreProcess.push_back(wb);
}

////////////////////////////////////////////////////////////////////////////////

boost::shared_mutex WorkQueue::s_mutex;

void WorkQueue::updateFromSongRouting()
{
	WorkQueue* newq = new WorkQueue(Song::get().m_routing);

	// Swap the new for the old
	{
		boost::unique_lock<boost::shared_mutex> lock(s_mutex);
		s = newq;
	}
}

void WorkQueue::setNull()
{
	boost::unique_lock<boost::shared_mutex> lock(s_mutex);
	s = NULL;
}

/////////////////////////////////////////////////////////////////////////////

QString quote(const QString& s)
{
	return QString('"') + s + QString('"');
}

void WorkQueue::dumpToDot(std::ostream& stream)
{
	stream << "digraph { node [shape=rectangle];" << std::endl;
	
	BOOST_FOREACH(const Ptr<WorkUnit::Base>& unit, m_units)
	{
		unit->dumpToDot(stream);
		unit->dumpDepsToDot(stream);
	}

	stream << "}" << std::endl;
}

void WorkUnit::Base::dumpToDot(std::ostream& stream)
{
	stream << quote(m_objectUuid.str()) << " [label=" << quote(typeid(*this).name()) << "];" << std::endl;
}

void WorkUnit::WorkMachine::dumpToDot(std::ostream& stream)
{
	QString label = QString("Work machine '%1'\\n(%2)").arg(m_machine->getName()).arg(typeid(*m_machine).name());

	stream << quote(m_objectUuid.str()) << " [label=" << quote(label) << "];" << std::endl;
}

void WorkUnit::FanIn::dumpToDot(std::ostream& stream)
{
	stream << quote(m_objectUuid.str()) << " [label=" << quote("Fan in") << "];" << std::endl;
}

void WorkUnit::FeedbackRead::dumpToDot(std::ostream& stream)
{
	stream << quote(m_objectUuid.str()) << " [label=" << quote("Feedback read") << "];" << std::endl;
	stream << quote(m_objectUuid.str()) << " -> " << quote(m_line->m_objectUuid.str())
		<< " [color=red, arrowhead=none];" << std::endl;
}

void WorkUnit::FeedbackWrite::dumpToDot(std::ostream& stream)
{
	stream << quote(m_objectUuid.str()) << " [label=" << quote("Feedback write") << "];" << std::endl;
	stream << quote(m_line->m_objectUuid.str()) << " -> " << quote(m_objectUuid.str())
		<< " [color=red, arrowhead=none];" << std::endl;
	stream << quote(m_line->m_objectUuid.str()) << " [color=red, shape=point];" << std::endl;
}

void WorkUnit::Chain::dumpToDot(std::ostream& stream)
{
	stream << "subgraph " << quote("cluster_" + m_objectUuid.str()) << " { rank=same; " << std::endl;
	QString label = QString("Chain\\nframes per work = %1").arg(m_maxFramesPerWork);
	stream << quote(m_objectUuid.str()) << " [label=" << quote(label) << "];" << std::endl;
	for (size_t u=0; u<m_units.size(); u++)
	{
		m_units[u]->dumpToDot(stream);
		if (u > 0)
		{
			stream	<< quote(m_units[u-1]->m_objectUuid.str()) << " -> " 
					<< quote(m_units[u]->m_objectUuid.str()) << std::endl;
		}
	}
	stream << "}" << std::endl;
}

void WorkUnit::Base::dumpDepsToDot(std::ostream& stream)
{
	BOOST_FOREACH(const Ptr<WorkUnit::Base>& dep, m_dependents)
	{
		stream << quote(m_objectUuid.str()) << " -> " << quote(dep->m_objectUuid.str()) << ";" << std::endl;
	}
}

