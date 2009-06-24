#pragma once

template<typename T> void insertIntoLinkedList(T** list, T* e)
{
	if (!*list) // list is empty
	{
		*list = e;
		e->next = NULL;
	}
	else if (e->time <= (*list)->time) // it can go at the front
	{
		e->next = *list;
		*list = e;
	}
	else
	{
		// Points to the element to insert e after
		T* before = *list;

		// Advance to the correct place in the list
		while (before->next && before->next->time < e->time) before = before->next;

		// Insert
		e->next = before->next;
		before->next = e;
	}
}

template<typename T> void mergeLinkedLists(T** a, T* b, bool steal)
{
	// Insert each element of b into a.
	// We know that successive elements have to be inserted after previous ones, so
	// at each stage we insert into the bit of the list after the element we just
	// inserted. Right? Right.

	// Initially points to the start of the list, subsequently points to the "next" of the
	// just inserted element
	T** aa = a;

	while (b)
	{
		// Store the next value for the loop, because b->next is about to be overwritten
		T* next = b->next;

		// The element to be inserted
		T* e = steal ? b : new T(*b);
		e->next = NULL;

		// Insert it, and update aa
		insertIntoLinkedList(aa, e);
		aa = &e->next;

		// Next loop iteration
		b = next;
	}
}

template<typename T> void deleteLinkedList(T* a)
{
	while (a)
	{
		T* next = a->next;
		delete a;
		a = next;
	}
}