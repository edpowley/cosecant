#pragma once

#include "sequence.h"

class Spattern : public Sequence::Pattern
{
	friend class SpatternEditor;

	Q_OBJECT

public:
	Spattern(Machine* mac, double length) : Pattern(mac), m_length(length) {}

	virtual double getLength() { return m_length; }

//	virtual void load(SongLoadContext& ctx, const QDomElement& el);
//	virtual void save(const QDomElement& el);

	virtual QUndoCommand* createUndoableForLengthChange(double newlength);

	class Note : public Object
	{
	public:

		double m_note, m_vel;
		double m_length;
	};

	typedef std::multimap<double, Ptr<Note> > NoteMap;
	NoteMap m_notes;

signals:
	void signalChange();

protected:
	double m_length;
};
