#pragma once

#include "machine.h"
#include "workbuffer.h"
#include "parameter.h"

class WorkQueue;

namespace WorkUnit
{
	class Base : public ObjectWithUuid
	{
		friend class Chain;
	public:
		Base(WorkQueue* q) : m_queue(q), m_depsAll(0), m_depsToGo(0) {}

		virtual int getMaxFramesPerWork() { return AudioIO::c_maxBufferSize; }

		void go(int numframes);

		void resetDeps() { m_depsToGo = m_depsAll; }

		// Only call depMM and ready if you have the unit queue mutex
		void depMM();
		bool ready() const { return m_depsToGo <= 0; }

		void addDependent(const Ptr<Base>& other);

		virtual void dumpToDot(std::ostream& stream);
		void dumpDepsToDot(std::ostream& stream);

	protected:
		WorkQueue* m_queue;

		virtual void work(int firstframe, int lastframe) = 0;

		int m_depsAll, m_depsToGo;
		std::vector< Ptr<Base> > m_dependents;
	};

	/////////////////////////////////////////////////////////////////////////

	class WorkMachine : public Base
	{
	public:
		WorkMachine(WorkQueue* q, const Ptr<Machine>& machine);
		virtual ~WorkMachine();

		std::vector< Ptr<WorkBuffer::Base> > m_inWorkBuffer, m_outWorkBuffer;
		PinBuffer *m_inPinBuffer, *m_outPinBuffer;
		PinBuffer m_eventPinBuffer;
		void updatePinBuffers();
		WorkContext m_workContext;

		virtual void dumpToDot(std::ostream& stream);

	protected:
		virtual void work(int firstframe, int lastframe);

		Ptr<Machine> m_machine;
		std::vector< Ptr<Pin> > m_inpins, m_outpins;

		struct ParamPinBuf
		{
			ParamTag tag;
			Ptr<Parameter::Scalar> param;
			Ptr<WorkBuffer::ParamControl> buf;
			bool doTimeUnitConversion;
			TimeUnit::e fromTimeUnit, toTimeUnit;
		};
		std::vector<ParamPinBuf> m_paramPinBufs;

		void updatePinBuffers(PinBuffer*& pinBuffers,
							  const std::vector< Ptr<WorkBuffer::Base> >& workBuffers);

		void sendParamChanges();
	};

	///////////////////////////////////////////////////////////////////////

	class FanIn : public Base
	{
	public:
		FanIn(WorkQueue* q, const Ptr<WorkBuffer::Base>& outbuf)
			: Base(q), m_outbuf(outbuf) {}

		void addInBuf(const Ptr<WorkBuffer::Base>& buf) { m_inbufs.push_back(buf); }

		virtual void dumpToDot(std::ostream& stream);

	protected:
		Ptr<WorkBuffer::Base> m_outbuf;
		std::vector< Ptr<WorkBuffer::Base> > m_inbufs;
		virtual void work(int firstframe, int lastframe);
	};

	////////////////////////////////////////////////////////////////////////

	class FeedbackRead : public Base
	{
	public:
		FeedbackRead(WorkQueue* q, const Ptr<DelayLine::Base>& line, const Ptr<WorkBuffer::Base>& buf)
			: Base(q), m_line(line), m_buf(buf) {}

		virtual int getMaxFramesPerWork() { return m_line->getLength(); }

		virtual void dumpToDot(std::ostream& stream);

	protected:
		Ptr<DelayLine::Base> m_line;
		Ptr<WorkBuffer::Base> m_buf;
		virtual void work(int firstframe, int lastframe);
	};

	class FeedbackWrite : public Base
	{
	public:
		FeedbackWrite(WorkQueue* q, const Ptr<DelayLine::Base>& line, const Ptr<WorkBuffer::Base>& buf)
			: Base(q), m_line(line), m_buf(buf) {}

		virtual int getMaxFramesPerWork() { return m_line->getLength(); }

		virtual void dumpToDot(std::ostream& stream);

	protected:
		Ptr<DelayLine::Base> m_line;
		Ptr<WorkBuffer::Base> m_buf;
		virtual void work(int firstframe, int lastframe);
	};

	////////////////////////////////////////////////////////////////////////

	class Chain : public Base
	{
	public:
		Chain(WorkQueue* q) : Base(q), m_maxFramesPerWork(AudioIO::c_maxBufferSize) {}

		void add(const Ptr<Base>& u)
		{
			m_units.push_back(u);
			m_maxFramesPerWork = min(m_maxFramesPerWork, u->getMaxFramesPerWork());
		}

		virtual void dumpToDot(std::ostream& stream);

	protected:
		std::vector< Ptr<Base> > m_units;
		virtual void work(int firstframe, int lastframe);
		int m_maxFramesPerWork;
	};
};
