#pragma once

/** A collection of convenience classes and functions. */
namespace CosecantHelper
{
	using namespace CosecantAPI;

	/** Convert a wide character string to a UTF-8 encoded string. This is a convenience wrapper for
		HostFunctions::toUtf8.
	*/
	inline std::string toUtf8(const std::wstring& str)
	{
		int bufsize = g_host->toUtf8(NULL, 0, str.c_str());
		std::vector<char> buf(bufsize);
		g_host->toUtf8(&buf.front(), bufsize, str.c_str());
		return std::string(&buf.front());
	}

	/** \class DebugPrint
	A convenience wrapper for HostFunctions::debugPrint. An instance of this class behaves much like an STL
	output stream. Note that nothing is actually printed until the instance is destroyed.

	Example usage:
	\code
	DebugPrint() << "The value of x is" << x;
	\endcode
	*/
	/** \fn bool DebugPrint::getSpace()
	Get whether a space is added between printed items. \sa setSpace
	*/
	/** \fn void DebugPrint::setSpace(bool space)
	Set whether a space is added between printed items. The default is \p true. For example:
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
	/** \fn template<typename T> DebugPrint& DebugPrint::operator<<(const T& v)
	Write an item to the stream. The item can be any type supported by \p std::ostream's << operator,
	and you can add support for your own types in the usual way. For example:
	\code
	std::ostream& operator<<(std::ostream& stream, const MyType& value)
	{
		return stream << "MyType(" << value.foo << ", " << value.bar << ")";
	}
	\endcode
	*/
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
	
	class StatusMessageBuilder
	{
	public:
		StatusMessageBuilder() : m_space(true) {}

		template<typename T> StatusMessageBuilder& operator<<(const T& v)
		{
			m_ss << v;
			if (m_space) m_ss << ' ';
			return *this;
		}

		std::string getMsg() const { return m_ss.str(); }

	protected:
		std::ostringstream m_ss;
		bool m_space;
	};

	class StatusMessage
	{
	public:
		StatusMessage(const StatusMessageBuilder& smb) { g_host->pushStatus(smb.getMsg().c_str()); }
		~StatusMessage() { g_host->popStatus(); }
	};

	/////////////////////////////////////////////////////////////////////////////

	/**	\class MutexLock
	An RAII locker for a machine's mutex. This saves you the effort of remembering to unlock the mutex. Simply
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
			
	Note that the mutex is \em recursive: it is safe to lock the mutex if it is already locked by this thread.
	*/
	/** \class MutexLock::Timeout
	This exception is thrown if the mutex lock in the constructor times out.
	*/
	/** \fn MutexLock::MutexLock(HostMachine* hm)
	The constructor. If the lock times out (because some other thread keeps the mutex locked for too
	long), an exception of class Timeout is thrown. Make sure that this eventuality will not put your machine
	into an inconsistent state.
	*/
	class MutexLock
	{
	public:
		class Timeout : public std::exception {};
		
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

	///////////////////////////////////////////////////////////////////////////////////
	
	class Blob
	{
	public:
		Blob() : m_dataHead(0) {}
		Blob(const void* data, unsigned int dataSize) : m_dataHead(0) { push(data, dataSize); }

		void* getData() { return &m_data[m_dataHead]; }
		unsigned int getDataSize() { return m_data.size() - m_dataHead; }

		void push(const void* data, unsigned int dataSize)
		{
			const char* p = reinterpret_cast<const char*>(data);
			for (unsigned int i=0; i<dataSize; i++,p++)
				m_data.push_back(*p);
		}

		template<typename T> void push(const T& data) { push(&data, sizeof(T)); }

		template<typename C> void pushString(const std::basic_string<C>& str)
		{
			push<unsigned int>(str.length());
			push(str.c_str(), str.length() * sizeof(C));
		}

		unsigned int pull(void* data, unsigned int dataSize)
		{
			char* p = reinterpret_cast<char*>(data);
			unsigned int i;
			for (i=0; i<dataSize && m_dataHead < m_data.size(); i++,p++,m_dataHead++)
				*p = m_data[m_dataHead];
			return i;
		}

		class BlobEmpty : public std::exception {};

		template<typename T> T pull()
		{
			T t;
			if (pull(&t, sizeof(T)) != sizeof(T))
				throw BlobEmpty();
			return t;
		}

		template<typename C> std::basic_string<C> pullString()
		{
			unsigned int len; len = pull<unsigned int>();
			std::basic_string<C> str;
			str.reserve(len);
			for (unsigned int i=0; i<len; i++)
				str.append(1, pull<C>());
			return str;
		}

	protected:
		std::vector<char> m_data;
		unsigned int m_dataHead;
	};
};
