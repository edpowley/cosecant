#pragma once

template<typename T> void vectorErase(std::vector<T>& v, const T& what, bool firstonly = false)
{
	for (std::vector<T>::iterator i = v.begin(); i != v.end(); )
	{
		if (*i == what)
		{
			i = v.erase(i);
			if (firstonly) return;
		}
		else
			++i;
	}
}

template<typename T> void vectorEraseFirst(std::vector<T>& v, const T& what)
{	vectorErase(v, what, true);   }

template<typename T> void vectorEraseAll(std::vector<T>& v, const T& what)
{	vectorErase(v, what, false);   }

/////////////////////////////////////////////////////////////////////////////

template<typename T, typename C> bool containerContains(const C& haystack, const T& needle)
{	
	for (C::const_iterator iter = haystack.begin(); iter != haystack.end(); ++iter)
	{
		if (*iter == needle)
			return true;
	}
	return false;
}

template<typename T, typename C> bool containerContainsUsingFind(const C& haystack, const T& needle)
{	return haystack.find(needle) != haystack.end();   }

template<typename T> bool containerContains(const std::set<T>& haystack, const T& needle)
{	return containerContainsUsingFind(haystack, needle);   }

template<typename T, typename U> bool containerContains(const std::map<T,U>& haystack, const T& needle)
{	return containerContainsUsingFind(haystack, needle);   }


template<typename C1, typename C2> bool containersIntersect(const C1& c1, const C2& c2)
{
	for (C1::const_iterator i = c1.begin(); i != c1.end(); ++i)
	{
		if (containerContains(c2, *i))
			return true;
	}
	return false;
}

/////////////////////////////////////////////////////////////////////////////

// Just a nicer class for timed mutex locks than boost provides
// (takes care of all that boost::system_time and boost::posix_time crap, and throws an exception
// instead of using a boolean)

ERROR_CLASS(MutexLockTimeout);

template<typename T> class TimeoutLock
{
public:
	TimeoutLock(T& mutex, unsigned int milliseconds)
		: m_mutex(mutex)
	{
		if (!m_mutex.timed_lock(boost::posix_time::milliseconds(milliseconds)))
			THROW_ERROR(MutexLockTimeout, "Mutex lock timeout");
	}

	~TimeoutLock()
	{
		m_mutex.unlock();
	}
		
protected:
	T& m_mutex;
};
