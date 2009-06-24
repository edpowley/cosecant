#include "stdafx.h"
#include "common.h"
#include "spattern.h"
#include "xmlutils.h"

void Spattern::load(SongLoadContext& ctx, xmlpp::Element* el)
{
	BOOST_FOREACH(xmlpp::Node* node, el->get_children("note"))
	{
		xmlpp::Element* child = dynamic_cast<xmlpp::Element*>(node);

		Ptr<Note> note = new Note;
		note->m_note	= getAttribute<double>(child, "note");
		note->m_vel		= getAttribute<double>(child, "vel");
		note->m_length	= getAttribute<double>(child, "length");

		m_notes.insert(std::make_pair(getAttribute<double>(child, "time"), note));
	}
}

void Spattern::save(xmlpp::Element* el)
{
	for (NoteMap::const_iterator iter = m_notes.begin(); iter != m_notes.end(); ++iter)
	{
		xmlpp::Element* child = el->add_child("note");
		setAttribute(child, "time",		iter->first);
		setAttribute(child, "note",		iter->second->m_note);
		setAttribute(child, "vel",		iter->second->m_vel);
		setAttribute(child, "length",	iter->second->m_length);
	}
}

QUndoCommand* Spattern::createUndoableForLengthChange(double newlength)
{
	return NULL;
}
