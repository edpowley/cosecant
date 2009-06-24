#pragma once

#include <map>
#include <string>
#include <sstream>
#include <iomanip>

#ifdef COSECANT_API_HOST
	class MachInfo;
	namespace ParamInfo { class Group; };
	namespace WorkBuffer { class Base; };
	class DllMachine;
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
			integer = 1 << 1,
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
	typedef ::MachInfo MachineInfo;
	typedef ::ParamInfo::Group ParamGroup;
	typedef ::DllMachine HostMachine;
	typedef xmlpp::Element XmlElement;
	class MiUndoable;
#else
	typedef void MachineInfo;
	typedef void ParamGroup;
	typedef void HostMachine;
	typedef void XmlElement;

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

	struct InfoCallbacks
	{
		unsigned int hostVersion;

		void (*setName)(MachineInfo* info, const char* name);
		void (*setTypeHint)(MachineInfo* info, MachineTypeHint::mt type);

		void (*addInPin )(MachineInfo* info, const char* name, SignalType::st type);
		void (*addOutPin)(MachineInfo* info, const char* name, SignalType::st type);

		void (*setParams)(MachineInfo* info, ParamGroup* params);
		
		ParamGroup* (*createParamGroup)(const char* name, ParamTag tag);
		void (*addRealParam)(ParamGroup* group, const char* name, ParamTag tag,
							double mini, double maxi, double def, unsigned long flags);
		void (*addTimeParam)(ParamGroup* group, const char* name, ParamTag tag,
							TimeUnit::unit internalUnit, double mini, double maxi, double def,
							unsigned int guiUnits, TimeUnit::unit guiDefaultUnit);
		void (*addEnumParam)(ParamGroup* group, const char* name, ParamTag tag,
							const char* items, unsigned int def);
		void (*addSubGroup)(ParamGroup* group, ParamGroup* sub);

		void (*addFlags)(MachineInfo*, unsigned int);
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

	struct Callbacks
	{
		unsigned int hostVersion;

		bool (*lockMutex)(HostMachine*);
		void (*unlockMutex)(HostMachine*);

		double (*getTicksPerFrame)();

		void (*addParamChange)(PinBuffer* buf, int time, ParamValue value);
		void (*addNoteEvent  )(PinBuffer* buf, int time, NoteEvent* ev);

		void (*doUndoable)(HostMachine*, MiUndoable*);

		void (*xmlSetAttribute_c)(XmlElement*, const char* name, const char* value);
		XmlElement* (*xmlAddChild)(XmlElement*, const char* tag);

		int (*xmlGetAttribute_c)(XmlElement*, const char* name, char* value, int value_size);
		int (*xmlGetTagName_c)(XmlElement*, char* value, int value_size);
		XmlElement* (*xmlGetFirstChild)(XmlElement*);
		XmlElement* (*xmlGetNextSibling)(XmlElement*);

		///////////////////////////////////////////////////////////////////////

		template<typename T>
		void xmlSetAttribute(XmlElement* el, const char* name, const T& value)
		{
			std::ostringstream ss;
			ss << value;
			xmlSetAttribute_c(el, name, ss.str().c_str());
		}

		template<typename T>
		T xmlGetAttribute(XmlElement* el, const char* name)
		{
			int numchars = xmlGetAttribute_c(el, name, NULL, -1);
			if (numchars == 0)
				throw LoadError(std::string("Attribute '") + name + std::string("' not found"));

			char* attr = new char[numchars];
			xmlGetAttribute_c(el, name, attr, numchars);
			std::string attrstr(attr);
			delete[] attr;

			T value;
			std::istringstream ss(attrstr);
			ss.exceptions(std::istringstream::failbit | std::istringstream::badbit);

			ss >> value;
			return value;
		}

		std::string xmlGetTagName(XmlElement* el)
		{
			int numchars = xmlGetTagName_c(el, NULL, -1);
			if (numchars > 0)
			{
				char* buf = new char[numchars];
				xmlGetTagName_c(el, buf, numchars);
				std::string ret(buf);
				delete[] buf;
				return ret;
			}
			else
				return std::string();
		}
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

#ifdef COSECANT_API_HOST
	class Mi;
	class MiPattern;
	class MiPatternEditor;
#else
	class Mi;

	class MiPattern
	{
		friend class MiPatternEditor;
	public:
		MiPattern(Mi* mi, int length) : m_mi(mi), m_length(length) {}
		virtual ~MiPattern() {}

		virtual void load(XmlElement* el) = 0;
		virtual void save(XmlElement* el) = 0;

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

#endif

	////////////////////////////////////////////////////////////////////

	struct MachineFunctions
	{
		unsigned int hostVersion;

		void (*destroy)(Mi*);
		void (*work)(Mi*, PinBuffer* inpins, PinBuffer* outpins, int first, int last);
		void (*changeParam)(Mi*, ParamTag tag, ParamValue value);
		void* (*noteOn)(Mi*, double note, double vel);
		void (*noteOff)(Mi*, void* note);
		void (*playPattern)(Mi*, void* track, MiPattern*, double pos);

		MiPattern* (*newPattern)(Mi*, int length);
		bool (*patLoad)(MiPattern*, XmlElement*);
		void (*patSave)(MiPattern*, XmlElement*);
		MiUndoable* (*patChangeLength)(MiPattern*, int length);
		void (*deletePattern)(MiPattern*);

		MiPatternEditor* (*createPatternEditor)(Mi*, WindowHandle parent, MiPattern*);
		void (*patedTakeFocus)(MiPatternEditor*);
		void (*patedKeyJazz)(MiPatternEditor*, KeyJazz::Type, KeyJazz::Note);
		void (*destroyPatternEditor)(MiPatternEditor*);

		void (*undoableDo)(MiUndoable*);
		void (*undoableUndo)(MiUndoable*);
		const char* (*undoableDescribe)(MiUndoable*);
		void (*undoableDestroy)(MiUndoable*);
	};

	////////////////////////////////////////////////////////////////////

	class MiFactory
	{
	public:
		virtual bool getInfo(MachineInfo* info, const InfoCallbacks* cb) = 0;
		virtual Mi* createMachine(HostMachine* mac, Callbacks* cb) = 0;
	};

	template<class TMiClass> class MiFactory_T : public MiFactory
	{
	public:
		virtual bool getInfo(MachineInfo* info, const InfoCallbacks* cb)
		{   return TMiClass::getInfo(info, cb);   }

		virtual Mi* createMachine(HostMachine* mac, Callbacks* cb)
		{   return new TMiClass(mac, cb);   }
	};

	// Your machine must implement this
	void populateMiFactories(std::map<std::string, MiFactory*>& factories);
};
