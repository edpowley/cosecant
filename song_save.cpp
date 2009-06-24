#include "stdafx.h"
#include "common.h"
#include "xmlutils.h"
#include "song.h"
#include "routing.h"
#include "sequence.h"
#include "machine.h"
#include "machinedll.h"
/*#include "notebookwindow.h"
#include "parameditor.h"
#include "routingeditor.h"
#include "sequenceeditor.h"
#include "perfmonitor.h"
#include "patterneditor.h"
*/

QString paramTagToString(ParamTag t)
{
	QString s;
	for (int i=0; i<4; i++)
	{
		unsigned char c = (unsigned char)((t >> ((3-i)*8)) & 0xFF);
		if (c < 32 || c >= 128 || c == '\\')
			s += QString::format("\\", std::hex, std::setfill('0'), std::setw(2), (int)c);
		else
			s += QString::format(c);
	}

	return s;
}

////////////////////////////////////////////////////////////////////////////////

void Song::doSave()
{
	if (m_savePath.empty())
		doSaveAs();
	else
		save(m_savePath);
}

void Song::doSaveAs()
{
	Gtk::FileChooserDialog dlg("Save As", Gtk::FILE_CHOOSER_ACTION_SAVE);
	dlg.set_do_overwrite_confirmation(true);
	dlg.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_REJECT);
	dlg.add_button(Gtk::Stock::OK, Gtk::RESPONSE_ACCEPT);

	Gtk::FileFilter filterDotCsc;
	filterDotCsc.add_pattern("*.csc");
	filterDotCsc.set_name("Cosecant song files (*.csc)");
	dlg.add_filter(filterDotCsc);
	dlg.set_filter(filterDotCsc);

	Gtk::FileFilter filterAll;
	filterAll.add_pattern("*");
	filterAll.set_name("All files (*.*)");
	dlg.add_filter(filterAll);

	if (!m_savePath.empty())
	{
		dlg.set_filename(Glib::filename_from_utf8(wstring_to_QString(m_savePath.file_string())));
	}

showSaveAsDialog:
	if (dlg.run() == Gtk::RESPONSE_ACCEPT)
	{
		bpath fname(QString_to_wstring(Glib::filename_to_utf8(dlg.get_filename())));
		if (fname.extension().empty())
		{
			fname.replace_extension(L".csc");
			if (boost::filesystem::exists(fname))
			{
				Gtk::MessageDialog msg(QString::compose(
					"File '%1' already exists. Do you want to replace it?", wstring_to_QString(fname.filename())
					), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_YES_NO, true);
				if (msg.run() == Gtk::RESPONSE_NO)
					goto showSaveAsDialog;
			}
		}

		m_savePath = fname;
		save(m_savePath);
	}
}

////////////////////////////////////////////////////////////////////////////////

void Song::save(const bpath& filepath)
{
	xmlpp::Document doc;
	xmlpp::Element* root = doc.create_root_node("song");
	root->set_attribute("creator", "BTDSys Cosecant ALPHA");
	root->set_attribute("version", "1");

	m_routing->save(root->add_child("routing"));

	m_sequence->save(root->add_child("sequence"));

	NotebookWindow::saveWorkspace(root->add_child("workspace"));

	doc.write_to_file_formatted(wstring_to_QString(filepath.file_string()));
}

//////////////////////////////////////////////////////////////////////////////

void Routing::save(xmlpp::Element* el)
{
	el->set_attribute("id", m_objectUuid.str());

	// Machines
	BOOST_FOREACH(const Ptr<Machine>& mac, m_machines)
	{
		mac->save(el->add_child("machine"));
	}

	// Connections
	BOOST_FOREACH(const Ptr<Machine>& mac, m_machines)
	{
		BOOST_FOREACH(const Ptr<Pin>& pin, mac->m_outpins)
		{
			BOOST_FOREACH(const Ptr<Connection>& conn, pin->m_connections)
			{
				conn->save(el->add_child("connection"));
			}
		}
	}
}

void Machine::save(xmlpp::Element* el)
{
	el->set_attribute("id", m_objectUuid.str());
	el->set_attribute("name", m_name);
	el->set_attribute("type", m_info->m_id);
	el->set_attribute("posx", QString::format(m_pos.x));
	el->set_attribute("posy", QString::format(m_pos.y));
	el->set_attribute("halfwidth", QString::format(m_halfsize.x));
	el->set_attribute("halfheight", QString::format(m_halfsize.y));

	// Parameter states
	typedef std::pair<ParamTag, ParamValue> paramtv;
	BOOST_FOREACH(const paramtv& tv, m_paramStates)
	{
		xmlpp::Element* child = el->add_child("param");
		child->set_attribute("tag", paramTagToString(tv.first));
		child->set_attribute("state", QString::format(tv.second));
	}

	// Pins
	BOOST_FOREACH(const Ptr<Pin>& pin, m_inpins)
	{
		pin->save(el->add_child("pin"));
	}

	BOOST_FOREACH(const Ptr<Pin>& pin, m_outpins)
	{
		pin->save(el->add_child("pin"));
	}

	// Patterns
	BOOST_FOREACH(const Ptr<Sequence::Pattern>& pat, m_patterns)
	{
		xmlpp::Element* child = el->add_child("pattern");
		setAttribute(child, "id", pat->m_objectUuid.str());
		setAttribute(child, "name", pat->m_name);
		setAttribute(child, "color", pat->m_color);
		setAttribute(child, "length", pat->getLength());
		pat->save(child);
	}
}

void Pin::save(xmlpp::Element* el)
{
	el->set_attribute("id", m_objectUuid.str());
	el->set_attribute("name", m_name);
	el->set_attribute("type", QString::format(m_type));
	el->set_attribute("pos", QString::format(m_pos));
	el->set_attribute("side", QString::format(m_side));

	if (m_direction == in)
	{
		if (m_isParamPin)
		{
			el->set_name("parampin");
			el->set_attribute("param", paramTagToString(m_paramTag));
		}
		else
			el->set_name("inpin");
	}
	else
		el->set_name("outpin");
}

void Connection::save(xmlpp::Element* el)
{
	el->set_attribute("id", m_objectUuid.str());
	el->set_attribute("pin1", m_pin1->m_objectUuid.str());
	el->set_attribute("pin2", m_pin2->m_objectUuid.str());
	if (m_feedback)
		el->set_attribute("feedback", "true");
}

////////////////////////////////////////////////////////////////////////////////

void Sequence::Seq::save(xmlpp::Element* el)
{
	setAttribute(el, "id", m_objectUuid.str());

	setAttribute(el, "loopstart", m_loopStart);
	setAttribute(el, "loopend", m_loopEnd);

	//m_masterTrack.save(el->add_child("mastertrack"));

	BOOST_FOREACH(const Ptr<Track>& track, m_tracks)
	{
		track->save(el->add_child("track"));
	}
}

void Sequence::Track::save(xmlpp::Element* el)
{
	setAttribute(el, "id", m_objectUuid.str());
	if (m_mac)
		setAttribute(el, "machine", m_mac->m_objectUuid.str());

	BOOST_FOREACH(const Ptr<Event>& sev, m_events)
	{
		sev->save(el->add_child("event"));
	}
}

void Sequence::Event::save(xmlpp::Element* el)
{
	setAttribute(el, "pattern", m_pattern->m_objectUuid.str());
	setAttribute(el, "starttime", m_startTime);
	setAttribute(el, "begin", m_begin);
	setAttribute(el, "end", m_end);
}

void DllMachine::Pattern::save(xmlpp::Element* el)
{
	m_instance->m_functions->patSave(m_ppat, el->add_child("machine_specific_data"));
}

///////////////////////////////////////////////////////////////////////////////

void NotebookWindow::saveWorkspace(xmlpp::Element* el)
{
	BOOST_FOREACH(NotebookWindow* w, s_windows)
	{
		w->save(el->add_child("window"));
	}
}

void NotebookWindow::save(xmlpp::Element* el)
{
	int x,y,w,h;
	get_position(x,y); get_size(w,h);
	setAttribute(el, "x", x);
	setAttribute(el, "y", y);
	setAttribute(el, "width", w);
	setAttribute(el, "height", h);

	Gdk::WindowState state = get_window()->get_state();
	if (state & Gdk::WINDOW_STATE_MAXIMIZED)
		setAttribute(el, "maximized", "true");

	BOOST_FOREACH(const Gtk::Notebook_Helpers::Page& pageiter, m_notebook.pages())
	{
		Gtk::Widget* pagew = pageiter.get_child();
		NotebookPage* page = dynamic_cast<NotebookPage*>(pagew);
		page->savePage(el->add_child("page"));
	}
}

void RoutingEditorPage::savePage(xmlpp::Element* el)
{
	el->set_name("routingeditor");
	el->set_attribute("routing", m_editor.getRouting()->m_objectUuid.str());
}

void PerfMonitor::savePage(xmlpp::Element* el)
{
	el->set_name("perfmonitor");
}

void ParamEditor::savePage(xmlpp::Element* el)
{
	el->set_name("parameditor");
	el->set_attribute("machine", m_mac->m_objectUuid.str());
}

void SequenceEditor::savePage(xmlpp::Element* el)
{
	el->set_name("sequenceeditor");
	el->set_attribute("sequence", m_seq->m_objectUuid.str());
}

void PatternEditor::savePage(xmlpp::Element* el)
{
	el->set_name("patterneditor");
	el->set_attribute("pattern", m_pattern->m_objectUuid.str());
}
