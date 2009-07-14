#pragma once

#include <map>
#include <string>
#include <sstream>
#include <iomanip>

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
			ticks    = 1 << 1,
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

#ifdef COSECANT_API_HOST
	typedef ::Machine HostMachine;
	class MiUndoable;
#else
	class HostMachine;

	class MiUndoable
	{
	public:
		virtual bool operator()() = 0;
		virtual bool undo() = 0;
		virtual const char* describe() = 0;
	};
#endif
	
	typedef void* WindowHandle;

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

	class LoadError : public std::exception
	{
	public:
		LoadError(const std::string& msg) : std::exception(msg.c_str()) {}
	};

	struct PlayingPattern
	{
		class MiPattern* pattern;
		double pos;
	};

	class Callbacks
	{
	public:
		virtual unsigned int getHostVersion() = 0;

		virtual bool lockMutex(HostMachine*) = 0;
		virtual void unlockMutex(HostMachine*) = 0;

		virtual double getTicksPerFrame() = 0;

		virtual void addParamChange(PinBuffer* buf, int time, ParamValue value) = 0;
		virtual void addNoteEvent  (PinBuffer* buf, int time, NoteEvent* ev) = 0;

		virtual void doUndoable(HostMachine*, MiUndoable*) = 0;
	};

	class MutexLock
	{
	public:
		class Timeout : public std::exception {};

		MutexLock(Callbacks* cb, HostMachine* mac) : m_cb(cb), m_mac(mac)
		{	if (!m_cb->lockMutex(m_mac)) throw Timeout();   }

		~MutexLock()
		{	m_cb->unlockMutex(m_mac);   }

	protected:
		Callbacks* m_cb;
		HostMachine* m_mac;
	};

	////////////////////////////////////////////////////////////////////

	namespace KeyJazz
	{
		enum Type { keyDown, keyRepeat, keyUp };
		typedef int Note;
		const Note noteOff = -1;
		const Note noteInvalid = -256;
	};

	class Mi;

	class MiPattern
	{
		friend class MiPatternEditor;
	public:
		MiPattern(Mi* mi, int length) : m_mi(mi), m_length(length) {}
		virtual ~MiPattern() {}

//		virtual void load(XmlElement* el) = 0;
//		virtual void save(XmlElement* el) = 0;

		virtual MiUndoable* createUndoableForLengthChange(int newlength) = 0;

	protected:
		Mi* m_mi;
		int m_length;
	};

	class MiPatternEditor
	{
	public:
		MiPatternEditor(MiPattern* pat, WindowHandle parent)
			: m_pat(pat), m_mi(pat->m_mi), m_parent(parent) {}
		virtual ~MiPatternEditor() {}

		virtual void takeFocus() = 0;

		virtual void keyJazz(KeyJazz::Type type, KeyJazz::Note note) {}

	protected:
		Mi* m_mi;
		MiPattern* m_pat;
		WindowHandle m_parent;
	};

	class Mi
	{
	public:
		Mi(HostMachine* mac, Callbacks* cb) : m_mac(mac), m_cb(cb) {}
		virtual ~Mi() {}

		virtual const char* getScript() { return NULL; }

		virtual void changeParam(ParamTag tag, ParamValue value) = 0;
		virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe) = 0;

		virtual void* noteOn(double note, double velocity) { return NULL; }
		virtual void noteOff(void* note) {}

		// If pattern == NULL, stop the current pattern
		// If track == NULL and pattern == NULL, stop all patterns
		virtual void playPattern(void* track, MiPattern* pattern, double pos) {}

		virtual MiPattern* createPattern(int length) { return NULL; }
		virtual MiPatternEditor* createPatternEditor(WindowHandle parent, MiPattern* pattern) { return NULL; }

		HostMachine* m_mac;
		Callbacks* m_cb;
	};

	////////////////////////////////////////////////////////////////////

	class MiFactory
	{
	public:
		virtual bool getInfo(MachineInfo* info, InfoCallbacks* cb) = 0;
		virtual Mi* createMachine(HostMachine* mac, Callbacks* cb) = 0;
	};

	template<class TMiClass> class MiFactory_T : public MiFactory
	{
	public:
		virtual bool getInfo(MachineInfo* info, InfoCallbacks* cb)
		{   return TMiClass::getInfo(info, cb);   }

		virtual Mi* createMachine(HostMachine* mac, Callbacks* cb)
		{   return new TMiClass(mac, cb);   }
	};

	// Your machine must implement this
	void populateMiFactories(std::map<std::string, MiFactory*>& factories);
};
