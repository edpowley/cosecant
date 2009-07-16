#pragma once

ERROR_CLASS(NullableIsNull);

template<typename T> class Nullable
{
public:
	Nullable() : m_null(true) {}
	Nullable(const T& val) : m_null(false), m_val(val) {}
	Nullable(const Nullable<T>& other) : m_null(other.m_null), m_val(other.m_val) {}

	bool isNull() { return m_null; }

	T get()
	{
		if (m_null)
			throw NullableIsNull();
		else
			return m_val;
	}

	T operator()() { return get(); }
	operator T() { return get(); }

	void set(const T& val) { m_null = false; m_val = val; }
	void setNull() { m_null = true; }

	Nullable& operator=(const T& val) { set(val); return *this; }
	Nullable& operator=(const Nullable<T>& other) { m_null = other.m_null; m_val = other.m_val; return *this; }

protected:
	bool m_null;
	T m_val;
};

