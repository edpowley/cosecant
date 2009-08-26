#pragma once

#include "cosecant_api.h"

class EventStream
{
public:
	typedef CosecantAPI::StreamEvent Event;

	class iterator : public QList<const Event*>::iterator
	{
	public:
		iterator() {}
		iterator(const QList<const Event*>::iterator& iter) : QList<const Event*>::iterator(iter) {}
		const Event* ptr() { return QList<const Event*>::iterator::operator*(); }
		const Event* operator->() { return ptr(); }
		const Event& operator*() { return *ptr(); }
	};

	class const_iterator : public QList<const Event*>::const_iterator
	{
	public:
		const_iterator() {}
		const_iterator(const QList<const Event*>::const_iterator& iter) : QList<const Event*>::const_iterator(iter) {}
		const Event* ptr() { return QList<const Event*>::const_iterator::operator*(); }
		const Event* operator->() { return ptr(); }
		const Event& operator*() { return *ptr(); }
	};

	bool empty() const { return m_list.empty(); }

	iterator		begin()			{ return m_list.begin(); }
	const_iterator	begin()	const	{ return m_list.begin(); }
	iterator		end()			{ return m_list.end(); }
	const_iterator	end()	const	{ return m_list.end(); }

	iterator		lowerBound(int time)		{ return indexToIterator(lowerBoundIndex(time)); }
	const_iterator	lowerBound(int time) const	{ return indexToIterator(lowerBoundIndex(time)); }
	iterator		upperBound(int time)		{ return indexToIterator(upperBoundIndex(time)); }
	const_iterator	upperBound(int time) const	{ return indexToIterator(upperBoundIndex(time)); }
	iterator		find(int time)				{ return indexToIterator(findIndex(time)); }
	const_iterator	find(int time)		 const	{ return indexToIterator(findIndex(time)); }

	iterator insert(const Event& ev);
	iterator erase(iterator iter);
	void clear();

	void offsetTimes(int offset);

protected:
	QList<const Event*> m_list;

	int binarySearch(int time) const;
	int lowerBoundIndex(int time) const;
	int upperBoundIndex(int time) const;
	int findIndex(int time) const;

	iterator indexToIterator(int i)
	{ if (i < 0) return end(); else return begin() + i; }

	const_iterator indexToIterator(int i) const
	{ if (i < 0) return end(); else return begin() + i; }
};
