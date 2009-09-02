#pragma once

#if defined(_WIN32)
#	define COSECANT_OS_WIN32
#else
#	error "Could not determine OS"
#endif

#if defined(min) || defined(max)
#	error "min or max is defined. If you include windows.h, do a '#define NOMINMAX' before it."
#endif

#ifndef COSECANT_API_NO_STDINT
#ifdef _MSC_VER
	// Visual C++ doesn't ship with stdint.h
	typedef	         __int8  int8_t;
	typedef unsigned __int8  uint8_t;
	typedef          __int32 int32_t;
	typedef unsigned __int32 uint32_t;
#else
#	include <stdint.h>
#endif
#endif

#include <sstream>
#include <vector>

#ifdef COSECANT_OS_WIN32
#	define COSECANT_PATHS_ARE_WIDE_STRINGS
#endif

// Your plugin must NOT define COSECANT_API_HOST
#ifdef COSECANT_API_HOST
#	include "hostapi.h"
#endif

/** The main namespace for the plugin API. */
namespace CosecantAPI
{
	/** The API version for this header file.
		\sa HostFunctions::getHostVersion */
	const uint32_t version = 1000;

	const uint32_t maxFramesPerBuffer = 1024;

#	ifdef COSECANT_PATHS_ARE_WIDE_STRINGS
		typedef wchar_t PathChar;
#	else
		typedef char PathChar;
#	endif

	class Mi;
	class MiPattern;

#	ifndef COSECANT_API_HOST
		class MiFactoryList;
		class HostMachine;
		class HostPattern;
		class SequenceTrack;
		class HostPinBuffer;
		class EventStreamIter;
#	endif

	typedef unsigned long ParamTag;

	//////////////////////////////////////////////////////////////////////////////////

	typedef uint8_t cbool;
	static const cbool cfalse = 0;
	static const cbool ctrue = 1;

	namespace MachineFlags
	{
		enum e
		{
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

		typedef uint8_t i;
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

		typedef uint8_t i;
	};

	namespace ParamScale
	{
		enum e { linear, logarithmic };
		typedef uint8_t i;
	};

	namespace ParamFlags
	{
		enum
		{
			noMin = 1 << 0,
			noMax = 1 << 1,
			noMinMax = noMin | noMax,
		};
		typedef uint32_t i;
	};

	/////////////////////////////////////////////////////////////////////////////

	namespace TimeUnit
	{
		enum e
		{
			none		= 0,
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

	struct StreamEvent_Note
	{
		void* id;
		double note;
		double vel;
	};

	struct StreamEvent_ParamChange
	{
		ParamTag tag;
		double value;
	};

	struct StreamEvent_Pattern
	{
		SequenceTrack* track;
		HostPattern* hostPattern;
		double pos;
	};

	struct StreamEvent_Custom
	{
		const char* type;
		void* data;
	};

	namespace StreamEventType
	{
		enum e
		{
			custom,
			noteOn,
			noteOff,
			paramChange,
			patternPlay,
			patternStop,
		};
		typedef uint8_t i;
	};

	struct StreamEvent
	{
		StreamEventType::i type;
		int32_t time;
		union
		{
			StreamEvent_Note note;
			StreamEvent_ParamChange paramChange;
			StreamEvent_Pattern pattern;
			StreamEvent_Custom custom;
		};
	};

	//////////////////////////////////////////////////////////////////////////

	namespace ParamType
	{
		enum e
		{
			tGroup, tReal, tInt, tTime, tEnum,
		};

		typedef uint8_t i;
	};

	struct ParamInfo
	{
		ParamType::i type;
		ParamTag tag;
		const char* name;
		ParamFlags::i flags;

		ParamInfo(ParamType::e t) : type(t), tag(0), name(NULL), flags(0) {}
	};

	struct ParamGroupInfo
	{
		ParamInfo p;
		const ParamInfo* const* params;

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
		int32_t min, max, def;

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
		uint32_t def;

		EnumParamInfo() : p(ParamType::tEnum), items(NULL), def(0) {}
	};

	///////////////////////////////////////////////////////////////////////////////

	namespace PinFlags
	{
		enum
		{
			breakOnEvent = 1 << 0,
		};
		typedef uint32_t i;
	};

	struct PinInfo
	{
		const char* name;
		SignalType::i type;
		PinFlags::i flags;

		PinInfo() : name(NULL), type(SignalType::monoAudio), flags(0) {}
		PinInfo(const char* name_, SignalType::e type_, PinFlags::i flags_ = 0)
			: name(name_), type(type_), flags(flags_) {}
	};

	struct MachineInfo
	{
		const char* defaultName;
		MachineTypeHint::i typeHint;
		MachineFlags::i flags;
		ParamGroupInfo params;
		const PinInfo* const* inPins;
		const PinInfo* const* outPins;

		MachineInfo() : defaultName(NULL), typeHint(MachineTypeHint::none), flags(0),
			inPins(NULL), outPins(NULL) {}
	};

	/////////////////////////////////////////////////////////////////////////////

	struct PinBuffer
	{
		HostPinBuffer* hostbuf;

		union
		{
			float* f;
			void* reserved[4];
		};
	};

	struct TimeInfo
	{
		double beatsPerSecond;
		int32_t beatsPerBar, beatsPerWholeNote;
		int32_t barsPerSmallGrid, smallGridsPerLargeGrid;
		int32_t samplesPerSecond;
	};

	struct WorkContext
	{
		const PinBuffer* ev;
		const PinBuffer* in;
		PinBuffer* out;
		int32_t firstframe, lastframe;
	};

	////////////////////////////////////////////////////////////////////

	struct PluginFunctions
	{
		void (*MiFactory_enumerate)(MiFactoryList* list);

		Mi* (*Mi_create)(const void* facUser, uint32_t facUserSize, HostMachine* hm);
		void (*Mi_destroy)(Mi* m);
		
		MachineInfo* (*Mi_getInfo)(Mi* m);
		void (*Mi_init)(Mi* m);
		
		void (*Mi_work)(Mi* m, const WorkContext* ctx);
		
		MiPattern* (*Mi_createPattern)(Mi* m, double length);
		void (*MiPattern_destroy)(MiPattern* p);
	};
	
	////////////////////////////////////////////////////////////////////

	/** \struct HostFunctions
	Callbacks provided by the host. Use these functions via the global g_host variable.
	*/
	/** \fn uint32_t (*HostFunctions::getHostVersion)()
	Get the plugin API version of the host.
	\sa version
	*/
	/** \fn void (*HostFunctions::debugPrint)(const char* msg)
	Write a string to the host's debug output. The CosecantHelper::DebugPrint class provides a convenient
	wrapper for this function.
	*/
	/** \fn void (*HostFunctions::registerMiFactory)(MiFactoryList* list, const char* id, const char* desc, void* user, uint32_t userSize)
	Register a machine provided by your plugin. Call this in your implementation of
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
	/** \fn const TimeInfo* (*HostFunctions::getTimeInfo)(HostMachine*)
	Get the TimeInfo structure associated with this machine.
	*/
	/** \fn cbool (*HostFunctions::lockMutex)(HostMachine*)
	Lock the machine's mutex. It is recommended that you use the MutexLock class instead of this function.
	\returns \p ctrue if the mutex was locked, \p cfalse if the lock timed out.
	*/
	/** \fn cbool (*HostFunctions::unlockMutex)(HostMachine*)
	Unlock the machine's mutex. It is recommended that you use the MutexLock class instead of this function.
	\returns \p ctrue
	*/
	struct HostFunctions
	{
		uint32_t (*getHostVersion)();

		void (*debugPrint)(const char* msg);
		void (*pushStatus)(const char* msg);
		void (*popStatus)();

		int32_t (*toUtf8)(char* buf, int32_t bufsize, const wchar_t* str);
		
		void (*registerMiFactory)(MiFactoryList* list,
			const char* id, const char* desc, void* user, uint32_t userSize);

		const TimeInfo* (*getTimeInfo)(HostMachine*);
				
		cbool (*lockMutex)(HostMachine*);
		cbool (*unlockMutex)(HostMachine*);

		void (*addParamChangeEvent)(PinBuffer* buf, int32_t time, double value);

		EventStreamIter* (*EventStream_begin)(const PinBuffer* buf);
		EventStreamIter* (*EventStream_end)(const PinBuffer* buf);
		EventStreamIter* (*EventStream_find)(const PinBuffer* buf, int32_t key);
		EventStreamIter* (*EventStream_lowerBound)(const PinBuffer* buf, int32_t key);
		EventStreamIter* (*EventStream_upperBound)(const PinBuffer* buf, int32_t key);
		EventStreamIter* (*EventStreamIter_copy)(EventStreamIter* iter);
		void (*EventStreamIter_destroy)(EventStreamIter* iter);
		void (*EventStreamIter_inc)(EventStreamIter* iter);
		void (*EventStreamIter_dec)(EventStreamIter* iter);
		int32_t (*EventStreamIter_deref)(EventStreamIter* iter, StreamEvent* ev, uint32_t evSize);
		cbool (*EventStreamIter_equal)(EventStreamIter* a, EventStreamIter* b);

		void (*iteratePaths)(
			const char* id, const char* name,
			void (*callback)(void* user, const PathChar* path),
			void* user);
	};
	
	/** A global instance of HostFunctions, initialised when your plugin is loaded.
		\sa implementation of csc_setHostFunctions in cosecant_api.cpp */
	extern HostFunctions* g_host;
	
	/////////////////////////////////////////////////////////////////////////////

#ifndef COSECANT_API_NO_CLASSES

	/** The base class for your machine's patterns. */
	class MiPattern
	{
	public:
		virtual ~MiPattern() {}
	};

	/** The base class for your machine. */
	class Mi
	{
	public:
		Mi(HostMachine* hm) : m_hm(hm) {}
		virtual ~Mi() {}

		virtual MachineInfo* getInfo() = 0;
		virtual void init() {}

		virtual void work(const WorkContext* ctx) = 0;

		virtual MiPattern* createPattern(double length) { return NULL; }

	protected:
		HostMachine* m_hm;
	};

#endif
};

#include "cosecant_api_helpers.h"

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
	CosecantAPI::Mi* createMachine(const void* facUser, uint32_t facUserSize, CosecantAPI::HostMachine* hm);
};
#endif
