#pragma once

#include "uuid.h"

/** Base class for all objects. Supports reference counting (in conjunction with Ptr). */
class Object : public QObject
{
	Q_OBJECT

public:
	Object()
	{
		m_refcountenable = true;
		m_refcount = 0;
	}

	virtual ~Object() {}

	/** Increment this object's reference count. This is called automatically if you are
		using Ptr.
		\sa decRef
	*/
	void incRef()
	{
		m_refcount.ref();
	}

	/** Decrement this object's reference count. This is called automatically if you are
		using Ptr. If reference counting is enabled (see enableRefCounting()) and the
		reference count hits zero, the object is deleted.
		\sa incRef
	*/
	void decRef()
	{
		if (!m_refcount.deref() && m_refcountenable)
			deleteLater();
	}

	/** Enable or disable reference counting. To be more precise, enable or disable
		deletion of the object when its reference count hits zero. For example, you must
		disable reference counting for objects allocated on the stack, or allocated by
		anything other than the ordinary <tt>new</tt> operator. Reference counting is
		enabled by default. */
	void enableRefCounting(bool enable=true) { m_refcountenable = enable; }

private:
	QAtomicInt m_refcount;
	bool m_refcountenable;

	Q_DISABLE_COPY(Object);
};

class ObjectWithUuid : public Object
{
public:
	Uuid m_objectUuid;
};

/** A "smart" pointer to an Object. It handles reference counting for you. The template
	parameter ObjectType should be a class derived from Object. */
template<class ObjectType> class Ptr
{
public:
	/** Construct from an existing C++ pointer. The usual way to construct a Ptr is like so:
			\code Ptr<SomeObjectClass> myptr(new SomeObjectClass(...)); \endcode

		Note that the default parameter value is <tt>NULL</tt>, so calling the constructor
		with no arguments will give a null pointer. Ptr can deal with null pointers,
		but as always you need to be careful when dereferencing them via operator ->() or
		operator ObjectType * ().
		
		Note also that this constructor is not
		declared <tt>explicit</tt>, so implicit conversions from ObjectType* are allowed.
	*/
	Ptr(ObjectType* p = NULL)
	{
		m_p = NULL;
		set(p);
	}

	/** Copy constructor. */
	Ptr(const Ptr<ObjectType>& other)
	{
		m_p = NULL;
		set(other.m_p);
	}

	/** Destructor. */
	~Ptr()
	{
		set(NULL);
	}

	/** Assignment operator from an existing Ptr. */
	Ptr<ObjectType>& operator=(const Ptr<ObjectType>& rhs)
	{ set(rhs.m_p); return *this; }

	/** Assignment operator from an existing C++ pointer. */
	Ptr<ObjectType>& operator=(ObjectType* rhs) { set(rhs); return *this; }

	/** Equality operator. */
	bool operator==(const Ptr<ObjectType>& other) const { return m_p == other.m_p; }

	/** Inequality operator. */
	bool operator!=(const Ptr<ObjectType>& other) const { return m_p != other.m_p; }

	/** Equality operator. */
	bool operator==(const ObjectType* p) const { return m_p == p; }

	/** Inequality operator. */
	bool operator!=(const ObjectType* p) const { return m_p != p; }

	ERROR_CLASS(NullPointerError);

	/** Member access. */
	ObjectType* operator->() const
	{
		if (m_p)
			return m_p;
		else
			THROW_ERROR(NullPointerError, "operator-> used on a null pointer");
	}

	/** Type cast to C++ pointer. */
	operator ObjectType*() const { return m_p; }
	ObjectType* c_ptr() const { return m_p; }

	/** Type cast to a Ptr to some other class. Will only work if the pointer can actually
		be accepted by the Ptr<OtherType> constructor. The most common case is when
		ObjectType is a subclass of OtherType, i.e. casting Ptr<DerivedClass> to
		Ptr<BaseClass>. */
	template<class OtherType> operator Ptr<OtherType>() const 
	{ return Ptr<OtherType>(m_p); }

private:
	ObjectType* m_p;
	void set(ObjectType* p)
	{
		if (p) p->incRef();
		if (m_p) m_p->decRef();
		m_p = p;
	}
};

// So that Ptr can be used as a key in a QHash
template<class ObjectType> uint qHash(const Ptr<ObjectType>& key)
{
	return qHash(key.c_ptr());
}

template<class ObjectType> class SingletonPtr
{
public:
	SingletonPtr()
	{
		m_p = NULL;
	}

	~SingletonPtr()
	{
		setNull();
	}

	ERROR_CLASS(SetError);

	void set(ObjectType* p)
	{
		if (!m_p)
			m_p = p;
		else
			THROW_ERROR(SetError, "set() can only be called once");
	}

	void setNull()
	{
		if (m_p) delete m_p;
		m_p = NULL;
	}

	ERROR_CLASS(NullPointerError);

	/** Member access. */
	ObjectType* operator->() const
	{
		if (m_p)
			return m_p;
		else
			THROW_ERROR(NullPointerError, "operator-> used on a null pointer");
	}

	/** Type cast to C++ pointer. */
	operator ObjectType*() const { return m_p; }
	ObjectType* c_ptr() const { return m_p; }

protected:
	ObjectType* m_p;
};