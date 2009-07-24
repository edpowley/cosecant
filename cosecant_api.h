#pragma once

#include <map>
#include <string>
#include <vector>

#ifdef COSECANT_API_HOST
	namespace WorkBuffer { class Base; };
	class Machine;
#endif

namespace CosecantAPI
{
	const unsigned int version = 1000;

	const unsigned int maxFramesPerBuffer = 1024;

	////////////////////////////////////////////////////////////////////

	namespace MachineFlags
	{
		enum
		{
			hasNoteTrigger = 1 << 0,
			createSequenceTrack = 1 << 1,
			hasCustomPatterns = 1 << 2,
			hasScript = 1 << 3,
		};
	};

	namespace MachineTypeHint
	{
		enum mt
		{
			none,
			master,
			generator,
			effect,
			control,
		};
	};

	namespace SignalType
	{
		enum st
		{
			monoAudio,
			stereoAudio,
			paramControl,
			noteTrigger,

			numSignalTypes,

			templated = 99,
			
			// Signaltypes below are for internal use only. Do not use them in machines.
			internal_seq = 100,
		};
	};

	////////////////////////////////////////////////////////////////////

	namespace ParamFlags
	{
		enum
		{
			logarithmic = 1 << 0,
		};
	};

	namespace TimeUnit
	{
		enum unit
		{
			seconds  = 1 << 0,
			beats    = 1 << 1,
			samples  = 1 << 2,
			hertz    = 1 << 3,
			fracfreq = 1 << 4, // 1.0 == sampling frequency
			notenum  = 1 << 5, // Like MIDI note numbers (A440 == 69)

			numUnits = 6
			// (1 << 0), ..., (1 << (numUnits-1)) are all valid units
		};
	};

	struct TimeValue
	{
		double value;
		TimeUnit::unit unit;

		TimeValue() : value(0), unit(TimeUnit::seconds) {}
		TimeValue(double v, TimeUnit::unit u) : value(v), unit(u) {}
	};

	////////////////////////////////////////////////////////////////////

	typedef unsigned long ParamTag;
	typedef double ParamValue;

	struct NoteEvent
	{
		void* id;
		double note;
		double vel;
		NoteEvent() { note = 0.0; vel = 0.0; }
	};
	
	////////////////////////////////////////////////////////////////////

	typedef void* ParamChangeBuffer;
	typedef void* NoteEventBuffer;

	struct PinBuffer
	{
		union
		{
			float* f;
		};

#ifdef COSECANT_API_HOST
		::WorkBuffer::Base* workbuffer;
		PinBuffer() : workbuffer(NULL) {}
		PinBuffer(::WorkBuffer::Base* wb) : workbuffer(wb) {}
#else
		void* reserved;
#endif
	};

	////////////////////////////////////////////////////////////////////

	namespace ParamInfo
	{
		class Base
		{
		public:
			virtual ~Base() {}

			virtual Base* copy() = 0;
		};

		class Group : public Base
		{
		public:
			virtual Group* copy() = 0;
			virtual Group* setName(const char* name) = 0;
			virtual Group* setTagMask(ParamTag mask) = 0;
			virtual Group* addParam(Base* param) = 0;
		};

		class Real : public Base
		{
		public:
			virtual Real* copy() = 0;
			virtual Real* setName(const char* name) = 0;
			virtual Real* setTag(ParamTag tag) = 0;
			virtual Real* setRange(ParamValue mn, ParamValue mx) = 0;
			virtual Real* setDefault(ParamValue def) = 0;
			virtual Real* addFlags(unsigned int flags) = 0;
		};

		class Int : public Base
		{
		public:
			virtual Int* copy() = 0;
			virtual Int* setName(const char* name) = 0;
			virtual Int* setTag(ParamTag tag) = 0;
			virtual Int* setRange(int mn, int mx) = 0;
			virtual Int* setDefault(int def) = 0;
		};

		class Time : public Base
		{
		public:
			virtual Time* copy() = 0;
			virtual Time* setName(const char* name) = 0;
			virtual Time* setTag(ParamTag tag) = 0;
			virtual Time* setRange(TimeValue mn, TimeValue mx) = 0;
			virtual Time* setDefault(TimeValue def) = 0;
			virtual Time* setInternalUnit(TimeUnit::unit unit) = 0;
			virtual Time* addDisplayUnits(unsigned int units) = 0;
			virtual Time* setDefaultDisplayUnit(TimeUnit::unit unit) = 0;

			Time* setRange(double mn, TimeUnit::unit mnu, double mx, TimeUnit::unit mxu)
			{ return setRange(TimeValue(mn, mnu), TimeValue(mx, mxu)); }
			Time* setDefault(double def, TimeUnit::unit unit)
			{ return setDefault(TimeValue(def, unit)); }
		};

		class Enum : public Base
		{
		public:
			virtual Enum* copy() = 0;
			virtual Enum* setName(const char* name) = 0;
			virtual Enum* setTag(ParamTag tag) = 0;
			virtual Enum* addItems(char separator, const char* text) = 0;
			virtual Enum* setDefault(int def) = 0;
			
			Enum* addItem(const char* text) { return addItems('\0', text); }
		};
	};

	class PinInfo
	{
	public:
		virtual PinInfo* setName(const char* name) = 0;
		virtual PinInfo* setType(SignalType::st type) = 0;
	};

	class MachineInfo
	{
	public:
		virtual MachineInfo* setName(const char* name) = 0;
		virtual MachineInfo* setTypeHint(MachineTypeHint::mt type) = 0;
		virtual MachineInfo* addFlags(unsigned int flags) = 0;
		virtual MachineInfo* addInPin (PinInfo* pin) = 0;
		virtual MachineInfo* addOutPin(PinInfo* pin) = 0;

		virtual ParamInfo::Group* getParams() = 0;
	};

	class InfoCallbacks
	{
	public:
		virtual unsigned int getHostVersion() = 0;

		virtual PinInfo* createPin() = 0;

		virtual ParamInfo::Group* createParamGroup() = 0;
		virtual ParamInfo::Real* createRealParam(ParamTag tag) = 0;
		virtual ParamInfo::Int*  createIntParam (ParamTag tag) = 0;
		virtual ParamInfo::Time* createTimeParam(ParamTag tag) = 0;
		virtual ParamInfo::Enum* createEnumParam(ParamTag tag) = 0;
	};

	////////////////////////////////////////////////////////////////////

	class Mi;

	namespace Script
	{
		class Value
		{
			friend class ValuePtr;

		public:
			virtual bool isValid() = 0;
			virtual bool isNull() = 0;

			virtual bool isBool() = 0;
			virtual bool toBool() = 0;

			virtual bool isNumber() = 0;
			virtual int toInt32() = 0;
			virtual double toInteger() = 0;
			virtual double toNumber() = 0;

			virtual bool isString() = 0;
			virtual int toString(char* buf, int bufsize) = 0;
			
			std::string toString()
			{
				int bufsize = toString(NULL, 0);
				std::vector<char> buf(bufsize, 0);
				toString(&buf.front(), bufsize);
				return std::string(&buf.front());
			}

		protected:
			virtual void incRef() = 0;
			virtual void decRef() = 0;
		};

		class ValuePtr
		{
		protected:
			Value* m_p;
			void set(Value* p) { if (p) p->incRef(); if (m_p) m_p->decRef(); m_p = p; }

		public:
			ValuePtr(const ValuePtr& other) : m_p(NULL) { set(other.m_p); }
			~ValuePtr() { set(NULL); }

#			ifdef COSECANT_API_HOST
				ValuePtr(Value* p) : m_p(NULL) { set(p); }
				Value* c_ptr() { return m_p; }
#			endif

			Value* operator->() { return m_p; }
		};	

		class Arguments
		{
		public:
			virtual int count() = 0;
			virtual ValuePtr operator[](int i) = 0;
		};
	};

	////////////////////////////////////////////////////////////////////

	class LoadError : public std::exception
	{
	public:
		LoadError(const std::string& msg) : std::exception(msg.c_str()) {}
	};

	struct TimeInfo
	{
		double beatsPerSecond;
		int beatsPerBar, beatsPerWholeNote;
		int barsPerSmallGrid, smallGridsPerLargeGrid;
		int samplesPerSecond;
	};

#ifdef COSECANT_API_HOST
	typedef ::Machine HostMachine;
#else
	class HostMachine;
#endif

	class Callbacks
	{
		friend class MutexLock;

	public:
		virtual unsigned int getHostVersion() = 0;
		virtual HostMachine* getHostMachine() = 0;

		virtual void debugPrint(const char* msg) = 0;

		virtual const TimeInfo* getTimeInfo() = 0;

		virtual void addScriptFunction(const char* name, int id) = 0;

		virtual void addParamChange(PinBuffer* buf, int time, ParamValue value) = 0;
		virtual void addNoteEvent  (PinBuffer* buf, int time, NoteEvent* ev) = 0;

		virtual Script::ValuePtr scriptValueNull() = 0;
		virtual Script::ValuePtr scriptValue(bool v) = 0;
		virtual Script::ValuePtr scriptValue(int v) = 0;
		virtual Script::ValuePtr scriptValue(double v) = 0;
		virtual Script::ValuePtr scriptValue(const char* v) = 0;

	protected:
		virtual bool lockMutex() = 0;
		virtual void unlockMutex() = 0;
	};

	class MutexLock
	{
	public:
		class Timeout : public std::exception {};

		MutexLock(Callbacks* cb) : m_cb(cb), m_locked(false)
		{	if (m_cb->lockMutex()) m_locked = true; else throw Timeout();   }

		~MutexLock()
		{	if (m_locked) m_cb->unlockMutex();   }

	protected:
		Callbacks* m_cb;
		bool m_locked;
	};

	////////////////////////////////////////////////////////////////////

	namespace KeyJazz
	{
		enum Type { keyDown, keyRepeat, keyUp };
		typedef int Note;
		const Note noteOff = -1;
		const Note noteInvalid = -256;
	};

	class MiPattern
	{
	public:
		MiPattern(double length) : m_length(length) {}

		double getLength() { return m_length; }

	protected:
		double m_length;
	};

	class Mi
	{
	public:
		Mi(Callbacks* cb) : m_cb(cb) {}
		virtual ~Mi() {}

		virtual void init() {}

		virtual const char* getScript() { return NULL; }
		virtual Script::ValuePtr callScriptFunction(int id, Script::Arguments* arg)
		{ return m_cb->scriptValueNull(); }

		virtual void changeParam(ParamTag tag, ParamValue value) = 0;
		virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe) = 0;

		virtual void* noteOn(double note, double velocity) { return NULL; }
		virtual void noteOff(void* note) {}

		// If pattern == NULL, stop the current pattern
		// If track == NULL and pattern == NULL, stop all patterns
		virtual void playPattern(void* track, MiPattern* pattern, double pos) {}

		virtual MiPattern* createPattern(double length) { return NULL; }

		Callbacks* m_cb;
	};

	////////////////////////////////////////////////////////////////////

	class MiFactory
	{
	public:
		virtual bool getInfo(MachineInfo* info, InfoCallbacks* cb) = 0;
		virtual Mi* createMachine(Callbacks* cb) = 0;
	};

	template<class TMiClass> class MiFactory_T : public MiFactory
	{
	public:
		virtual bool getInfo(MachineInfo* info, InfoCallbacks* cb)
		{   return TMiClass::getInfo(info, cb);   }

		virtual Mi* createMachine(Callbacks* cb)
		{   return new TMiClass(cb);   }
	};

	// Your machine must implement this
	void populateMiFactories(std::map<std::string, MiFactory*>& factories);
};
