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

namespace CosecantAPI
{
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
#	endif

	typedef unsigned long ParamTag;

#	if defined(COSECANT_OS_WIN32)
#		define COSECANT_TAG(x) (x)
#	else
#		error "COSECANT_TAG not defined"
#	endif

	//////////////////////////////////////////////////////////////////////////////////

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

	struct Variant
	{
		enum Type
		{
			tNull, tInt, tDouble, tString,
		};
		
		unsigned char type;
		
		union
		{
			int dInt;
			double dDouble;
			const char* dString;
		};
		
		Variant() : type(tNull) {}
		
		Variant(int v)			: type(tInt)	{ dInt = v; }
		Variant(double v)		: type(tDouble)	{ dDouble = v; }
		Variant(const char* v)	: type(tString)	{ dString = v; }
	};
	
	struct PinBuffer
	{
		void* reserved;

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
		void (*Mi_destroy)(Mi*);
		
		MachineInfo* (*Mi_getInfo)(Mi*);
		void (*Mi_init)(Mi*);
		
		void (*Mi_changeParam)(Mi*, ParamTag tag, double value);
		void (*Mi_work)(Mi*, PinBuffer* inpins, PinBuffer* outpins, int firstframe, int lastframe);
		MiNote* (*Mi_noteOn)(Mi*, double pitch, double velocity);
		void (*Mi_noteOff)(Mi*, MiNote*);
		
		Variant (*Mi_callScriptFunction)(Mi*, int id, const ScriptValue** args, int numargs);
		
		MiPattern* (*MiPattern_create)(Mi*, double length);
		void (*MiPattern_destroy)(MiPattern*);
		void (*Mi_playPattern)(Mi*, SequenceTrack* track, MiPattern* pattern, double startpos);
	};
	
	////////////////////////////////////////////////////////////////////

	struct HostFunctions
	{
		unsigned int (*getHostVersion)();
		void (*debugPrint)(const char* msg);
		
		void (*registerMiFactory)(MiFactoryList* list,
			const char* id, const char* desc, void* user, unsigned int userSize);

		const TimeInfo* (*getTimeInfo)(HostMachine*);
		
		void (*registerScriptFunction)(HostMachine*, const char* name, int id);
		
		bool (*lockMutex)(HostMachine*);
		bool (*unlockMutex)(HostMachine*);
	};
	
	extern HostFunctions* g_host;
	
	/////////////////////////////////////////////////////////////////////////////

#ifndef COSECANT_API_NO_CLASSES

	class MiNote
	{
	public:
		virtual void noteOff() = 0;
	};

	class MiPattern
	{
	public:
		virtual void play(SequenceTrack* track, double startpos) = 0;
		virtual void stop(SequenceTrack* track) = 0;
	};

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

		virtual Variant callScriptFunction(int id, const ScriptValue** args, int numargs)
		{ return Variant(); }

		virtual MiPattern* createPattern(double length) { return NULL; }

	protected:
		HostMachine* m_hm;
	};

	////////////////////////////////////////////////////////////////////////////

	class DebugPrint
	{
	public:
		DebugPrint() : m_space(true) {}
		~DebugPrint() { g_host->debugPrint(m_ss.str().c_str()); }
		
		bool getSpace() { return m_space; }
		void setSpace(bool space) { m_space = space; }
		
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
	
	class MutexLock
	{
	public:
		class Timeout : public std::exception {};
		
		MutexLock(HostMachine* hm) : m_hm(hm)
		{
			m_locked = g_host->lockMutex(m_hm);
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
namespace CosecantPlugin
{
	// Your plugin must implement these
	void enumerateFactories(CosecantAPI::MiFactoryList* list);
	CosecantAPI::Mi* createMachine(const void* facUser, unsigned int facUserSize, CosecantAPI::HostMachine* hm);
};
#endif
