#pragma once

#if defined(_WIN32)
#	define COSECANT_OS_WIN32
#else
#	error "Could not determine OS"
#endif

#if defined(min) || defined(max)
#	error "min or max is defined. If you include windows.h, do a '#define NOMINMAX' first."
#endif

#include <sstream>

// Your plugin must NOT define COSECANT_API_HOST
#ifdef COSECANT_API_HOST
#	include "hostapi.h"
#endif

/** The main namespace for the plugin API. */
namespace CosecantAPI
{
	/** The API version for this header file.
		\sa HostFunctions::getHostVersion */
	const unsigned int version = 1000;

	const unsigned int maxFramesPerBuffer = 1024;

	class Mi;
	class MiNote;
	class MiPattern;

#	ifndef COSECANT_API_HOST
		class MiFactoryList;
		class HostMachine;
		class ScriptValue;
		class SequenceTrack;
		class HostPinBuffer;
#	endif

	typedef unsigned long ParamTag;

#	if defined(COSECANT_OS_WIN32)
#		define COSECANT_TAG(x) (x)
#	else
#		error "COSECANT_TAG not defined"
#	endif

	//////////////////////////////////////////////////////////////////////////////////

	typedef unsigned char cbool;
	static const cbool cfalse = 0;
	static const cbool ctrue = 1;

	namespace MachineFlags
	{
		enum e
		{
			hasNoteTrigger			= 1 << 0,
			createSequenceTrack		= 1 << 1,
			hasCustomPatterns		= 1 << 2,
		};

		typedef unsigned long i;
	};

	namespace MachineTypeHint
	{
		enum e
		{
			none,
			master, generator, effect, control,
		};

		typedef unsigned char i;
	};

	namespace SignalType
	{
		enum e
		{
			monoAudio, stereoAudio,
			paramControl, noteTrigger,

			numSignalTypes,

			templated = 99,
			
			// Signaltypes below are for internal use only. Do not use them in machines.
			internal_seq = 100,
		};

		typedef unsigned char i;
	};

	namespace ParamScale
	{
		enum e { linear, logarithmic };
		typedef unsigned char i;
	};

	/////////////////////////////////////////////////////////////////////////////

	namespace TimeUnit
	{
		enum e
		{
			seconds		= 1 << 0,
			beats		= 1 << 1,
			samples		= 1 << 2,
			hertz		= 1 << 3,
			fracfreq	= 1 << 4, // 1.0 == sampling frequency
			notenum		= 1 << 5, // Like MIDI note numbers (A440 == 69)

			numUnits	= 6
			// (1 << 0), ..., (1 << (numUnits-1)) are all valid units
		};

		typedef unsigned long i;
	};

	struct TimeValue
	{
		double value;
		TimeUnit::i unit;

		TimeValue() : value(0), unit(TimeUnit::seconds) {}
		TimeValue(double v, TimeUnit::e u) : value(v), unit(u) {}
		void set(double v, TimeUnit::e u) { value = v; unit = u; }
	};

	//////////////////////////////////////////////////////////////////////////

	namespace ParamType
	{
		enum e
		{
			tGroup, tReal, tInt, tTime, tEnum,
		};

		typedef unsigned char i;
	};

	struct ParamInfo
	{
		ParamType::i type;
		ParamTag tag;
		const char* name;

		ParamInfo(ParamType::e t) : type(t), tag(0), name(NULL) {}
	};

	struct ParamGroupInfo
	{
		ParamInfo p;
		const ParamInfo** params;

		ParamGroupInfo() : p(ParamType::tGroup), params(NULL) {}
	};

	struct RealParamInfo
	{
		ParamInfo p;
		ParamScale::i scale;
		double min, max, def;

		RealParamInfo() : p(ParamType::tReal), scale(ParamScale::linear), min(0), max(1), def(0) {}
	};

	struct IntParamInfo
	{
		ParamInfo p;
		int min, max, def;

		IntParamInfo() : p(ParamType::tInt), min(0), max(1), def(0) {}
	};

	struct TimeParamInfo
	{
		ParamInfo p;
		TimeValue min, max, def;
		TimeUnit::i internalUnit, displayUnits, defaultDisplayUnit;

		TimeParamInfo() : p(ParamType::tTime), internalUnit(0), displayUnits(0), defaultDisplayUnit(0) {}
	};

	struct EnumParamInfo
	{
		ParamInfo p;
		const char** items;
		unsigned int def;

		EnumParamInfo() : p(ParamType::tEnum), items(NULL), def(0) {}
	};

	///////////////////////////////////////////////////////////////////////////////

	struct PinInfo
	{
		const char* name;
		SignalType::i type;
	};

	struct MachineInfo
	{
		const char* defaultName;
		MachineTypeHint::i typeHint;
		MachineFlags::i flags;
		ParamGroupInfo params;
		const PinInfo** inPins;
		const PinInfo** outPins;
		const char* script;

		MachineInfo() : defaultName(NULL), typeHint(MachineTypeHint::none), flags(0),
			inPins(NULL), outPins(NULL), script(NULL) {}
	};

	/////////////////////////////////////////////////////////////////////////////

	struct PinBuffer
	{
		HostPinBuffer* hostbuf;

		union
		{
			float* f;
		};
	};

	struct TimeInfo
	{
		double beatsPerSecond;
		int beatsPerBar, beatsPerWholeNote;
		int barsPerSmallGrid, smallGridsPerLargeGrid;
		int samplesPerSecond;
	};

	////////////////////////////////////////////////////////////////////

	struct PluginFunctions
	{
		void (*MiFactory_enumerate)(MiFactoryList* list);

		Mi* (*Mi_create)(const void* facUser, unsigned int facUserSize, HostMachine* hm);
		void (*Mi_destroy)(Mi* m);
		
		MachineInfo* (*Mi_getInfo)(Mi* m);
		void (*Mi_init)(Mi* m);
		
		void (*Mi_changeParam)(Mi* m, ParamTag tag, double value);
		void (*Mi_work)(Mi* m, PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);
		MiNote* (*Mi_noteOn)(Mi* m, double pitch, double velocity);
		void (*MiNote_off)(MiNote* n);
		
		ScriptValue* (*Mi_callScriptFunction)(Mi* m, int id, const ScriptValue** args, int numargs);
		
		MiPattern* (*Mi_createPattern)(Mi* m, double length);
		void (*MiPattern_destroy)(MiPattern* p);
		void (*MiPattern_play)(MiPattern* p, SequenceTrack* track, double startpos);
		void (*MiPattern_stop)(MiPattern* p, SequenceTrack* track);
	};
	
	////////////////////////////////////////////////////////////////////

	/** Callbacks provided by the host. Use these functions via the global g_host variable. */
	struct HostFunctions
	{
		/** Get the plugin API version of the host.
			\sa version */
		unsigned int (*getHostVersion)();

		/** Write a string to the host's debug output. The DebugPrint class provides a convenient wrapper
			for this function. */
		void (*debugPrint)(const char* msg);
		
		/** Register a machine provided by your plugin. Call this in your implementation of
			CosecantPlugin::enumerateFactories.
			\param list the argument to CosecantPlugin::enumerateFactories
			\param id a unique machine identifier, e.g. "btdsys/awesomesynth"
			\param desc a human-readable machine name, e.g. "BTDSys AwesomeSynth"
			\param user a pointer to data that will be passed to your CosecantPlugin::createMachine function.
				Note that the data is copied, and in general the actual pointer you pass here will be different
				from the pointer passed back to you. If you do not need any data, this can be \p NULL.
			\param userSize the length, in bytes, of the data pointed to by \p user. If \p user points to a string,
				remember to allow for the null terminator. Pass \p 0 if and only if user is \p NULL.
			*/
		void (*registerMiFactory)(MiFactoryList* list,
			const char* id, const char* desc, void* user, unsigned int userSize);

		/** Get the TimeInfo structure associated with this machine. */
		const TimeInfo* (*getTimeInfo)(HostMachine*);
		
		/** Register a function that can be called from your machine's script. A function named \p name will be
			added to your main script object's \p cscFunctions member; calling this function will call your
			Mi::callScriptFunction to be called.
			\param name the name of the function. This must be a valid QtScript identifier.
			\param id the integer identifier passed to Mi::callScriptFunction
			*/
		void (*registerScriptFunction)(HostMachine*, const char* name, int id);
		
		/** Lock the machine's mutex. It is recommended that you use the MutexLock class instead of this function.
			\returns \p ctrue if the mutex was locked, \p cfalse if the lock timed out. */
		cbool (*lockMutex)(HostMachine*);

		/** Unlock the machine's mutex. It is recommended that you use the MutexLock class instead of this function.
			\returns \p ctrue
		*/
		cbool (*unlockMutex)(HostMachine*);

		void (*addParamChangeEvent)(PinBuffer* buf, int time, double value);

		cbool (*ScriptValue_isNull)(const ScriptValue*);
		cbool (*ScriptValue_isValid)(const ScriptValue*);

		cbool (*ScriptValue_isNumber)(const ScriptValue*);
		int (*ScriptValue_toInt)(const ScriptValue*);
		double (*ScriptValue_toDouble)(const ScriptValue*);

		cbool (*ScriptValue_isString)(const ScriptValue*);
		int (*ScriptValue_toString)(const ScriptValue*, char* buf, int bufsize);

		cbool (*ScriptValue_isArray)(const ScriptValue*);
		unsigned int (*ScriptValue_getArrayLength)(const ScriptValue*);

		/** Get the array element at the specified index. Note that you take ownership of the return value,
			so it is your responsibility to call ScriptValue_destroy() when you are finished with it.
			\param sv the array
			\param index the index into the array
			\returns the index'th element of sv. If there is no such element, an invalid value is returned. */
		const ScriptValue* (*ScriptValue_getArrayElement)(const ScriptValue* sv, unsigned int index);

		Mi* (*ScriptValue_toMi)(const ScriptValue*);
		MiPattern* (*ScriptValue_toMiPattern)(const ScriptValue*);

		ScriptValue* (*ScriptValue_createNull)();
		ScriptValue* (*ScriptValue_createInvalid)();
		ScriptValue* (*ScriptValue_createInt)(int v);
		ScriptValue* (*ScriptValue_createDouble)(double v);
		ScriptValue* (*ScriptValue_createString)(const char* v);
		void (*ScriptValue_destroy)(const ScriptValue*);

		/** Create a new QtScript Array object.
			\param length the initial length (number of elements) of the array. Note that the array can be expanded
				by inserting elements beyond its current length. If you do not know the array length in advance, it
				is acceptable to pass 0 here.
			\returns a pointer to the new object, which you own. */
		ScriptValue* (*ScriptValue_createArray)(unsigned int length);

		/** Set the array element at the specified index. \p value is copied and the copy is inserted into the array,
			so you may safely destroy \p value after calling this function. In fact, the \p takeOwnership parameter
			allows this function to destroy \p value so you don't have to.
			\param arr an array, previously created with ScriptValue_createArray()
			\param index the index into the array
			\param value the new value of the index'th element of arr
			\param takeOwnership if \p ctrue, this function will destroy \p value before returning. */
		void (*ScriptValue_setArrayElement)(
			ScriptValue* arr, unsigned int index, ScriptValue* value, cbool takeOwnership);
	};
	
	/** A global instance of HostFunctions, initialised when your plugin is loaded.
		\sa implementation of csc_setHostFunctions in cosecant_api.cpp */
	extern HostFunctions* g_host;
	
	/////////////////////////////////////////////////////////////////////////////

#ifndef COSECANT_API_NO_CLASSES

	class MiNote
	{
	public:
		virtual void noteOff() = 0;
	};

	/** The base class for your machine's patterns. */
	class MiPattern
	{
	public:
		/** Start this pattern playing.
			\param track the sequence track on which this pattern plays
			\param startpos the starting position in beats. Do \em not assume that this is in the range [0, length).
				Because of the way Cosecant calculates pattern start times, it is more likely to be in the range
				(-beats_per_sample, length+beats_per_sample), but do not assume this either. */
		virtual void play(SequenceTrack* track, double startpos) = 0;

		/** Stop this pattern playing.
			\param track the sequence track on which this pattern plays
		*/
		virtual void stop(SequenceTrack* track) = 0;
	};

	/** The base class for your machine. */
	class Mi
	{
	public:
		Mi(HostMachine* hm) : m_hm(hm) {}
		virtual ~Mi() {}

		virtual MachineInfo* getInfo() = 0;
		virtual void init() {}

		virtual void changeParam(ParamTag tag, double value) = 0;
		virtual void work(PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe) = 0;
		virtual MiNote* noteOn(double pitch, double velocity) { return NULL; }

		/** Your machine's script can call this.
			\param id the integer function id, as passed to HostFunctions::registerScriptFunction
			\param args the arguments passed to the function. This array is NULL-terminated, i.e.
				<tt>args[numargs] == NULL</tt>. The host owns the arguments; you should not store or destroy them.
			\param numargs the number of arguments passed to the function
			\returns a new script value, probably created by one of the \p ScriptValue_create functions in
				HostFunctions. The host takes ownership of the return value and destroys it, so you should not
				store it. Returning a \p NULL pointer is the same as returning a null script value.
		*/
		virtual ScriptValue* callScriptFunction(int id, const ScriptValue** args, int numargs)
		{ return NULL; }

		virtual MiPattern* createPattern(double length) { return NULL; }

	protected:
		HostMachine* m_hm;
	};

	////////////////////////////////////////////////////////////////////////////

	/** A convenience wrapper for HostFunctions::debugPrint. An instance of this class behaves much like an STL
		output stream. Note that nothing is actually printed until the instance is destroyed.

		Example usage:
		\code
DebugPrint() << "The value of x is" << x;
		\endcode
	*/
	class DebugPrint
	{
	public:
		DebugPrint() : m_space(true) {}
		~DebugPrint() { g_host->debugPrint(m_ss.str().c_str()); }
		
		/** Get whether a space is added between printed items. \sa setSpace */
		bool getSpace() { return m_space; }

		/** Set whether a space is added between printed items. The default is \p true. For example:
			\code
int answer = 42;
{
	DebugPrint d;
	d.setSpace(true);
	d << "The answer is" << answer; // prints "The answer is 42"
}
{
	DebugPrint d;
	d.setSpace(false);
	d << "The answer is" << answer; // prints "The answer is42"
}
			\endcode
		*/
		void setSpace(bool space) { m_space = space; }
		
		/** Write an item to the stream. The item can be any type supported by \p std::ostringstream,
			and you can add support for your own types in the same way. */
		template<typename T> DebugPrint& operator<<(const T& v)
		{
			m_ss << v;
			if (m_space) m_ss << ' ';
			return *this;
		}
		
	protected:
		std::ostringstream m_ss;
		bool m_space;
	};
	
	/**	A RAII locker for a machine's mutex. This saves you the effort of remembering to unlock the mutex. Simply
		create an instance of this class when you want to lock the mutex, and the mutex will be unlocked once that
		instance falls out of scope.
		\code
{
	MutexLock lock(m_hm);
	// Now the mutex is locked
	doSomething();
}
// Now the mutex is unlocked
		\endcode
		
		Note that the mutex is recursive: it is safe to lock the mutex if it is already locked by this thread. */
	class MutexLock
	{
	public:
		/** This exception is thrown if the mutex lock in the constructor times out. */
		class Timeout : public std::exception {};
		
		/** The constructor. If the lock times out (because some other thread keeps the mutex locked for too
			long), an exception of class Timeout is thrown. Make sure that this eventuality will not put your machine
			into an inconsistent state. */
		MutexLock(HostMachine* hm) : m_hm(hm)
		{
			m_locked = (g_host->lockMutex(m_hm) != cfalse);
			if (!m_locked) throw Timeout();
		}
		
		~MutexLock() { if (m_locked) g_host->unlockMutex(m_hm); }
	
	protected:
		HostMachine* m_hm;
		bool m_locked;
	};

#endif

};

#ifdef COSECANT_OS_WIN32
#	define COSECANT_EXPORT(RETURN_TYPE) extern "C" __declspec(dllexport) RETURN_TYPE
#else
#	define COSECANT_EXPORT(RETURN_TYPE) extern "C" RETURN_TYPE
#endif

#ifndef COSECANT_API_HOST
/** Functions that must be implemented by your plugin. The implementation of csc_getPluginFunctions in
	cosecant_api.cpp returns pointers to these functions to the host. */
namespace CosecantPlugin
{
	/** Get a list of the machines provided by this plugin. Here, your implementation should call
		CosecantAPI::HostFunctions::registerMiFactory one or more times.
		If your plugin is a loader for another plugin format, this is the appropriate place to scan for plugins. */
	void enumerateFactories(CosecantAPI::MiFactoryList* list);

	/** Create a machine. Here you should create and return a new instance of your machine class (using \p new).
		\param facUser a pointer to a copy of the user data passed to CosecantAPI::HostFunctions::registerMiFactory,
			or \p NULL if no data was passed
		\param facUserSize the size, in bytes, of the data pointed to by \p facUser
		\param hm a pointer to the host machine, to be passed to your machine class's constructor
		\returns a \p new instance of your machine class */
	CosecantAPI::Mi* createMachine(const void* facUser, unsigned int facUserSize, CosecantAPI::HostMachine* hm);
};
#endif
