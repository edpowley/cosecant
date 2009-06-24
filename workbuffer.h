#pragma once

#include "audioio.h"
#include "machinfo.h"
#include "delayline.h"
#include "sequenceevent.h"

namespace WorkBuffer
{
	enum Flags
	{
		needsPreProcess = 1 << 0,
	};

	/////////////////////////////////////////////////////////////////////////////

	class Base : public Object
	{
	public:
		virtual unsigned int getFlags() const = 0;

		virtual PinBuffer getPinBuffer() = 0;

		virtual void clearAll() = 0;
		virtual void clear(int firstframe, int lastframe) = 0;
		virtual void copy(WorkBuffer::Base* other, int firstframe, int lastframe) = 0;
		virtual void mix (WorkBuffer::Base* other, int firstframe, int lastframe) = 0;

		virtual void preProcess() {}
	};

	///////////////////////////////////////////////////////////////////////////

	class Audio : public Base
	{
	public:
		Audio(unsigned int nChannels);

		static const unsigned int s_flags = 0;
		virtual unsigned int getFlags() const { return s_flags; }

		virtual PinBuffer getPinBuffer()
		{ PinBuffer pb(this); pb.f = &m_data[0]; return pb; }

		virtual void clearAll();
		virtual void clear(int firstframe, int lastframe);
		virtual void copy(WorkBuffer::Base* other, int firstframe, int lastframe);
		virtual void mix (WorkBuffer::Base* other, int firstframe, int lastframe);

		const unsigned int m_nChannels;
		std::vector<float> m_data;
	};

	class MonoAudio : public Audio
	{
	public:
		MonoAudio() : Audio(1) {}

		static Ptr<DelayLine::Base> createDelayLine(int length)
		{ return new DelayLine::Audio(length, 1); }
	};

	class StereoAudio : public Audio
	{
	public:
		StereoAudio() : Audio(2) {}

		static Ptr<DelayLine::Base> createDelayLine(int length)
		{ return new DelayLine::Audio(length, 2); }
	};

	///////////////////////////////////////////////////////////////////////////

	class ParamControl : public Base
	{
	public:
		static const unsigned int s_flags = needsPreProcess;
		virtual unsigned int getFlags() const { return s_flags; }

		ParamControl() : m_lastValue(0) {}

		virtual PinBuffer getPinBuffer()
		{ PinBuffer pb(this); return pb; }

		static Ptr<DelayLine::Base> createDelayLine(int length) { return new DelayLine::ParamControl(length); }

		virtual void preProcess();
		virtual void clearAll();
		virtual void clear(int firstframe, int lastframe);
		virtual void copy(WorkBuffer::Base* other, int firstframe, int lastframe);
		virtual void mix (WorkBuffer::Base* other, int firstframe, int lastframe);

		std::map<int, ParamValue> m_data;
		ParamValue m_lastValue;
	};

	//////////////////////////////////////////////////////////////////////////

	class SequenceEvents : public Base
	{
	public:
		static const unsigned int s_flags = needsPreProcess;
		virtual unsigned int getFlags() const { return s_flags; }

		SequenceEvents() {}

		virtual PinBuffer getPinBuffer()
		{ PinBuffer pb(this); return pb; }

		static Ptr<DelayLine::Base> createDelayLine(int length) { return new DelayLine::SequenceEvents(length); }

		virtual void preProcess() { clearAll(); }
		virtual void clearAll();
		virtual void clear(int firstframe, int lastframe);
		virtual void copy(WorkBuffer::Base* other, int firstframe, int lastframe);
		virtual void mix (WorkBuffer::Base* other, int firstframe, int lastframe);

		typedef std::multimap<int, Ptr<SequenceEvent::Base> > EventMap;

		EventMap m_data;
	};

	///////////////////////////////////////////////////////////////////////////

	class Factory_Base : public Object
	{
	public:
		virtual Ptr<Base> create() = 0;
		virtual Ptr<DelayLine::Base> createDelayLine(int length) = 0;
		virtual unsigned int getFlags() = 0;
		virtual const char* getDescription() = 0;
	};

	template<class T> class Factory : public Factory_Base
	{
	public:
		Factory(const char* description) : m_description(description) {}
		virtual Ptr<Base> create() { return new T; }
		virtual Ptr<DelayLine::Base> createDelayLine(int length) { return T::createDelayLine(length); }
		virtual unsigned int getFlags() { return T::s_flags; }
		virtual const char* getDescription() { return m_description; }

	protected:
		const char* m_description;
	};

	extern std::map<SignalType::st, Ptr<Factory_Base> > s_factories;

	class FactoriesInitialiser
	{
	public:
		FactoriesInitialiser();
	};

	inline Ptr<Base> create(SignalType::st signaltype)
	{
		return s_factories[signaltype]->create();
	}

	inline unsigned int getFlags(SignalType::st signaltype)
	{
		return s_factories[signaltype]->getFlags();
	}

	inline const char* getDescription(SignalType::st signaltype)
	{
		return s_factories[signaltype]->getDescription();
	}

};