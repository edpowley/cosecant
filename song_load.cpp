#include "stdafx.h"
#include "common.h"
#include "xmlutils.h"
//#include "dlg_file.h"
#include "song.h"
/*#include "notebookwindow.h"
#include "routingeditor.h"
#include "sequenceeditor.h"
#include "perfmonitor.h"
#include "parameditor.h"
*/
#include "dllmachine.h"

unsigned char unHex(QChar qc)
{
	char c = qc.toAscii();
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return c - 'A';
	else if (c >= 'a' && c <= 'f')
		return c - 'a';
	else
		THROW_ERROR(Error, "Invalid hex digit");
}

ParamTag paramTagFromString(const QString& str)
{
	ParamTag tag = 0;
	for (QString::const_iterator i = str.begin(); i != str.end(); ++i)
	{
		unsigned char c;
		if (*i == '\\')
		{
			unsigned char d1 = unHex(*(++i));
			unsigned char d2 = unHex(*(++i));
			c = (d1 << 4) | d2;
		}
		else
		{
			c = i->toAscii();
		}

		tag <<= 8;
		tag |= c;
	}

	return tag;
}

/////////////////////////////////////////////////////////////////////

/*
void Song::doOpen()
{
	FileDialog dlg(FileDialog::open, "Open");
	dlg.addFilter("Cosecant song files (*.csc)", "*.csc");
	dlg.addFilter("All files", "*");
	bpath fname = dlg.run(m_savePath);

	if (!fname.empty())
		if (load(fname))
			m_savePath = fname;
}
*/

//////////////////////////////////////////////////////////////////////

xmlpp::Element* getUniqueElement(xmlpp::Element* parent, const QString& name, bool throwIfMissing = true)
{
	const xmlpp::Node::NodeList nodes = parent->get_children(q2ustring(name));
	if (nodes.empty())
	{
		if (throwIfMissing)
		{
			THROW_ERROR(SongLoadError, QString("'%1' element at line %2 is missing a '%3' element")
				.arg(parent->get_name().c_str()).arg(parent->get_line()).arg(name));
		}
		else
			return NULL;
	}
	else if (nodes.size() > 1)
	{
		THROW_ERROR(SongLoadError, QString("'%1' element at line %2 has more than one '%3' element")
			.arg(parent->get_name().c_str()).arg(parent->get_line()).arg(name));
	}
	else
	{
		return dynamic_cast<xmlpp::Element*>(nodes.front());
	}
}

bool Song::load(const bpath& filepath)
{
//	NotebookWindow::closeAllPages();
	clear();

	xmlpp::DomParser parser;
	parser.set_substitute_entities();
//	try
	{
		parser.parse_file(wstring_to_ustring( filepath.file_string() ));
		if (!parser) THROW_ERROR(Error, "!parser");
		xmlpp::Element* root = parser.get_document()->get_root_node();

		SongLoadContext ctx;

		xmlpp::Element* routingEl = getUniqueElement(root, "routing");
		m_routing = new Routing(ctx, routingEl);
		connect(
			m_routing, SIGNAL(signalTopologyChange()),
			this, SLOT(updateWorkQueue())
		);

		xmlpp::Element* sequenceEl = getUniqueElement(root, "sequence");
		m_sequence = new Sequence::Seq(ctx, sequenceEl);

//		xmlpp::Element* wspaceEl = getUniqueElement(root, "workspace");
//		NotebookWindow::loadWorkspace(ctx, wspaceEl);

		m_routing->signalTopologyChange();

		if (!ctx.m_nonFatalErrors.empty())
		{
			QString msg = "Errors occurred while loading. Some parts of the song may not have been loaded.\n";
			BOOST_FOREACH(const QString& err, ctx.m_nonFatalErrors)
			{
				msg += "\n"; msg += err;
			}

			QMessageBox msgbox;
			msgbox.setText(msg);
			msgbox.setIcon(QMessageBox::Warning);
			msgbox.exec();
		}

		return true;
	}
/*	catch (const std::exception& err)
	{
		QMessageBox msgbox;
		msgbox.setText(QString("Error loading song file: \n") + err.what());
		msgbox.setIcon(QMessageBox::Critical);
		msgbox.exec();
		return false;
	}
*/
}

//////////////////////////////////////////////////////////////////////////

Routing::Routing(SongLoadContext& ctx, xmlpp::Element* parent)
{
	m_objectUuid = getAttribute<Uuid>(parent, "id");
	ctx.setObject(m_objectUuid, this);

	ctorCommon();

	BOOST_FOREACH(xmlpp::Node* node, parent->get_children("machine"))
	{
		xmlpp::Element* el = dynamic_cast<xmlpp::Element*>(node);
		QString type = getAttribute<QString>(el, "type");
		Ptr<Machine> machine;
		try
		{
			Ptr<MachineFactory> factory = MachineFactory::get(type);
			machine = factory->createMachine();
		}
		catch (const MachineFactory::BadIdError&)
		{
			ctx.m_nonFatalErrors.push_back(QString(
				"Machine '%1' (%2) is missing, and has been replaced with a dummy machine.")
				.arg(type).arg(getAttribute<QString>(el, "name")) );
			machine = MachineFactory::get("builtin/dummy")->createMachine();
		}
		machine->load(ctx, el);
		addMachine(machine);
	}

	BOOST_FOREACH(xmlpp::Node* node, parent->get_children("connection"))
	{
		Ptr<Connection> conn = new Connection(ctx, dynamic_cast<xmlpp::Element*>(node));
		addConnection(conn);
	}
}

void DummyMachine::load(SongLoadContext& ctx, xmlpp::Element* el)
{
	BOOST_FOREACH(xmlpp::Node* pinNode, el->get_children("inpin"))
	{
		xmlpp::Element* pinEl = dynamic_cast<xmlpp::Element*>(pinNode);
		Ptr<Pin> pin = new Pin(this, Pin::in,
			static_cast<SignalType::st>(getAttribute<int>(pinEl, "type")));
		m_inpins.push_back(pin);
	}
	BOOST_FOREACH(xmlpp::Node* pinNode, el->get_children("outpin"))
	{
		xmlpp::Element* pinEl = dynamic_cast<xmlpp::Element*>(pinNode);
		Ptr<Pin> pin = new Pin(this, Pin::out,
			static_cast<SignalType::st>(getAttribute<int>(pinEl, "type")));
		m_outpins.push_back(pin);
	}

	Machine::load(ctx, el);
}

void Machine::load(SongLoadContext& ctx, xmlpp::Element* el)
{
	m_objectUuid = getAttribute<Uuid>(el, "id");
	ctx.setObject(m_objectUuid, this);

	m_name = getAttribute<QString>(el, "name");
	m_pos.setX(getAttribute<double>(el, "posx"));
	m_pos.setY(getAttribute<double>(el, "posy"));
	m_halfsize.setX(getAttribute<double>(el, "halfwidth"));
	m_halfsize.setY(getAttribute<double>(el, "halfheight"));

	size_t pinIndex = 0;
	BOOST_FOREACH(xmlpp::Node* pinNode, el->get_children("inpin"))
	{
		if (pinIndex >= m_inpins.size()) THROW_ERROR(SongLoadError, "Too many inpins");
		m_inpins[pinIndex]->load(ctx, dynamic_cast<xmlpp::Element*>(pinNode));
		++ pinIndex;
	}

	pinIndex = 0;
	BOOST_FOREACH(xmlpp::Node* pinNode, el->get_children("outpin"))
	{
		if (pinIndex >= m_outpins.size()) THROW_ERROR(SongLoadError, "Too many outpins");
		m_outpins[pinIndex]->load(ctx, dynamic_cast<xmlpp::Element*>(pinNode));
		++ pinIndex;
	}

	BOOST_FOREACH(xmlpp::Node* pinNode, el->get_children("parampin"))
	{
		xmlpp::Element* pinEl = dynamic_cast<xmlpp::Element*>(pinNode);

		Ptr<Pin> pin = new Pin(this, Pin::in, SignalType::paramControl);
		pin->m_isParamPin = true;
		pin->m_paramTag = paramTagFromString(getAttribute<QString>(pinEl, "param"));
		pin->load(ctx, pinEl);
		addPin(pin);
	}

	BOOST_FOREACH(xmlpp::Node* paramNode, el->get_children("param"))
	{
		xmlpp::Element* paramEl = dynamic_cast<xmlpp::Element*>(paramNode);
		ParamTag tag = paramTagFromString(getAttribute<QString>(paramEl, "tag"));
		double state = getAttribute<double>(paramEl, "state");
		m_paramStates[tag] = state;
		m_paramChanges[tag] = state;
	}

	BOOST_FOREACH(xmlpp::Node* patNode, el->get_children("pattern"))
	{
		xmlpp::Element* patEl = dynamic_cast<xmlpp::Element*>(patNode);
		createPattern(ctx, patEl);
	}
}

void Pin::load(SongLoadContext& ctx, xmlpp::Element* el)
{
	m_objectUuid = getAttribute<Uuid>(el, "id");
	ctx.setObject(m_objectUuid, this);

	m_pos = getAttribute<float>(el, "pos");
	m_side = (Pin::Side)getAttribute<int>(el, "side");
}

Connection::Connection(SongLoadContext& ctx, xmlpp::Element* el)
{
	m_objectUuid = getAttribute<Uuid>(el, "id");
	ctx.setObject(m_objectUuid, this);

	try
	{
		m_feedback = getAttribute<bool>(el, "feedback");
	}
	catch (const MissingAttributeError&)
	{
		m_feedback = false;
	}

	m_pin1 = ctx.getObject<Pin>(getAttribute<Uuid>(el, "pin1"));
	m_pin2 = ctx.getObject<Pin>(getAttribute<Uuid>(el, "pin2"));
	if (!m_pin1 || !m_pin2)
		THROW_ERROR(SongLoadError, QString("Connection pin not found at line %1").arg(el->get_line()));
}

/////////////////////////////////////////////////////////////////////////

Sequence::Seq::Seq(SongLoadContext& ctx, xmlpp::Element* parent)
{
	m_objectUuid = getAttribute<Uuid>(parent, "id");
	ctx.setObject(m_objectUuid, this);

	ctorCommon();

	BOOST_FOREACH(xmlpp::Node* trackNode, parent->get_children("track"))
	{
		xmlpp::Element* trackEl = dynamic_cast<xmlpp::Element*>(trackNode);
		Ptr<Track> track = new Track(ctx, trackEl);
		m_tracks.push_back(track);
	}

	m_playPos = 0;
	m_playing = false;
	m_loopStart = getAttribute<double>(parent, "loopstart");
	m_loopEnd = getAttribute<double>(parent, "loopend");
	// tick/sample = (tick/minute) / (second/minute) / (sample/second)
	m_ticksPerFrame = 120.0 / 60.0 / 44100.0;
}

Sequence::Track::Track(SongLoadContext& ctx, xmlpp::Element* parent)
{
	m_objectUuid = getAttribute<Uuid>(parent, "id");
	ctx.setObject(m_objectUuid, this);

	ctorCommon();

	m_mac = ctx.getObjectOrThrow<Machine>(getAttribute<Uuid>(parent, "machine"));

	BOOST_FOREACH(xmlpp::Node* evNode, parent->get_children("event"))
	{
		xmlpp::Element* evEl = dynamic_cast<xmlpp::Element*>(evNode);
		m_events.insert(new Event(ctx, evEl));
	}
}

Sequence::Clip::Event(SongLoadContext& ctx, xmlpp::Element* el)
{
	m_pattern = ctx.getObjectOrThrow<Pattern>(getAttribute<Uuid>(el, "pattern"));
	m_startTime = getAttribute<double>(el, "starttime");
	m_begin = getAttribute<double>(el, "begin");
	m_end = getAttribute<double>(el, "end");
}

Ptr<Sequence::Pattern> Machine::createPattern(SongLoadContext& ctx, xmlpp::Element* el)
{
	Ptr<Sequence::Pattern> pat = createPattern(getAttribute<double>(el, "length"));
	m_patterns.push_back(pat);

	pat->m_objectUuid = getAttribute<Uuid>(el, "id");
	ctx.setObject(pat->m_objectUuid, pat);
	
	pat->m_name = getAttribute<QString>(el, "name");
	pat->m_color = getAttribute<QColor>(el, "color");

	pat->load(ctx, el);

	return pat;
}

/*
void DllMachine::Pattern::load(SongLoadContext& ctx, xmlpp::Element* el)
{
	if (!m_instance->m_functions->patLoad(m_ppat, getUniqueElement(el, "machine_specific_data")))
		THROW_ERROR(SongLoadError, "Machine's patLoad function failed");
}
*/

/////////////////////////////////////////////////////////////////////////
/*
void NotebookWindow::loadWorkspace(SongLoadContext& ctx, xmlpp::Element* parent)
{
	xmlpp::Node::NodeList windowNodes = parent->get_children("window");
	for (xmlpp::Node::NodeList::iterator iter = windowNodes.begin(); iter != windowNodes.end(); ++iter)
	{
		xmlpp::Element* el = dynamic_cast<xmlpp::Element*>(*iter);
		NotebookWindow* win = new NotebookWindow;
		win->load(ctx, el);
	}
}

void NotebookWindow::load(SongLoadContext& ctx, xmlpp::Element* el)
{
	int x = getAttribute<int>(el, "x");
	int y = getAttribute<int>(el, "y");
	int w = getAttribute<int>(el, "width");
	int h = getAttribute<int>(el, "height");

	bool maximise;
	try
	{
		maximise = getAttribute<bool>(el, "maximized");
	}
	catch (MissingAttributeError)
	{
		maximise = false;
	}

	move(x,y);
	if (maximise)
		maximize();
	else
		resize(w,h);

	BOOST_FOREACH(xmlpp::Node* pageNode, el->get_children())
	{
		xmlpp::Element* pageEl = dynamic_cast<xmlpp::Element*>(pageNode);
		if (!pageEl) continue;

		NotebookPage* page = NotebookPage::loadPage(ctx, pageEl);
		if (page) addPage(Gtk::manage(page));
	}
}

NotebookPage* NotebookPage::loadPage(SongLoadContext& ctx, xmlpp::Element* el)
{
	QString tag = el->get_name();
	if (tag == "routingeditor")
	{
		Routing* routing = ctx.getObjectOrThrow<Routing>(getAttribute<Uuid>(el, "routing"));
		return new RoutingEditorPage(routing);
	}
	else if (tag == "sequenceeditor")
	{
		Sequence::Seq* seq = ctx.getObjectOrThrow<Sequence::Seq>(getAttribute<Uuid>(el, "sequence"));
		return new SequenceEditor(seq);
	}
	else if (tag == "parameditor")
	{
		Machine* mac = ctx.getObjectOrThrow<Machine>(getAttribute<Uuid>(el, "machine"));

		if (mac->m_parameditor) THROW_ERROR(SongLoadError, "Machine already has a param editor");
		mac->m_parameditor = new ParamEditor(mac);
		return mac->m_parameditor;
	}
	else if (tag == "perfmonitor")
	{
		return new PerfMonitor;
	}
	else if (tag == "patterneditor")
	{
		Sequence::Pattern* patt = ctx.getObjectOrThrow<Sequence::Pattern>(getAttribute<Uuid>(el, "pattern"));

		if (patt->m_editor) THROW_ERROR(SongLoadError, "Pattern already has an editor");
		patt->m_editor = patt->m_mac->createPatternEditor(patt);
		return patt->m_editor;
	}
	else
	{
		ctx.m_nonFatalErrors.push_back(QString::compose("Unknown page type '%1' at line %2", tag, el->get_line()));
		return NULL;
	}
}
*/
