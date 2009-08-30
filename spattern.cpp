#include "stdafx.h"
#include "common.h"
#include "spattern.h"
using namespace Spattern;

#include "spatternmachine.h"

Pattern::Pattern(SpatternMachine* mac, double length)
: m_mac(mac), Sequence::Pattern(mac, length)
{
}

///////////////////////////////////////////////////////////////////////////////////

Note::Note(Pattern* pattern, double start, double length, double note)
: m_pattern(pattern), m_start(start), m_length(length), m_note(note)
{
}

void Pattern::addNote(const Ptr<Note>& note)
{
	m_notes.insert(note->getStart(), note);
	signalNoteAdded(note);
}

void Pattern::removeNote(const Ptr<Note>& note)
{
	m_notes.remove(note->getStart(), note);
	signalNoteRemoved(note);
}

