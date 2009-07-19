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

template<typename K, typename V> class MyMap : public std::map<K,V>
{
public:
	ERROR_CLASS(KeyError);

	bool contains(const K& key) const
	{
		return find(key) != end();
	}

	V& get(const K& key)
	{
		iterator iter = find(key);
		if (iter != end())
			return iter->second;
		else
			throw KeyError("Missing key");
	}

	const V& get(const K& key) const
	{
		const_iterator iter = find(key);
		if (iter != end())
			return iter->second;
		else
			throw KeyError("Missing key");
	}
};

/////////////////////////////////////////////////////////////////////////////

ERROR_CLASS(MutexLockTimeout);

class MutexLockerWithTimeout
{
public:
	MutexLockerWithTimeout(QMutex* mutex, int milliseconds)
		: m_mutex(mutex), m_locked(false)
	{
		if (m_mutex->tryLock(milliseconds))
			m_locked = true;
		else
			throw MutexLockTimeout("Mutex lock timeout");
	}

	~MutexLockerWithTimeout()
	{
		if (m_locked) m_mutex->unlock();
	}
		
protected:
	QMutex* m_mutex;
	bool m_locked;
};
