#include "stdafx.h"
#include "common.h"
#include "song.h"

#include "xmlutils.h"
#include "builtinmachines.h"
#include "parameter.h"
#include "spattern.h"

extern zlib_filefunc_def g_zipFileFuncs; // zipfilefuncs.cpp

class SongLoadFile
{
public:
	SongLoadFile(const QString& path)
	{
		m_p = unzOpen2(path.toUtf8(), &g_zipFileFuncs);
		if (!m_p) throw SongLoadError(Song::tr("Unable to read file '%1'").arg(path));
	}

	~SongLoadFile()
	{
		if (m_p) unzClose(m_p);
	}

	QByteArray readFile(const QString& name)
	{
		int err;
		err = unzLocateFile(m_p, name.toAscii(), 2); // 2 = not case sensitive
		if (err != UNZ_OK)
			throw SongLoadError(QString("Error code %1 in unzLocateFile").arg(err));

		err = unzOpenCurrentFile(m_p);
		if (err != UNZ_OK)
			throw SongLoadError(QString("Error code %1 in unzOpenCurrentFile").arg(err));

		QByteArray ret;
		const int bufsize = 16384;
		char buf[bufsize];
		err = 1;
		while (err > 0)
		{
			err = unzReadCurrentFile(m_p, buf, bufsize);
			if (err < 0)
			{
				unzCloseCurrentFile(m_p);
				throw SongLoadError(QString("Error code %1 in unzReadCurrentFile").arg(err));
			}

			if (err > 0) // err is the number of bytes read
			{
				int oldsize = ret.size();
				ret.resize(oldsize + err);
				memcpy(ret.data() + oldsize, buf, err);
			}
		}

		err = unzCloseCurrentFile(m_p);
		if (err != UNZ_OK)
			throw SongLoadError(QString("Error code %1 in unzCloseCurrentFile").arg(err));

		return ret;
	}

protected:
	unzFile m_p;
};


void Song::load(const QString& filepath)
{
	SongLoadFile f(filepath);

	QString errorMsg; int errorLine, errorColumn;
	QDomDocument doc;
	if (!doc.setContent( f.readFile("song.xml"), false, &errorMsg, &errorLine, &errorColumn ))
	{
		throw SongLoadError(tr("XML parse error '%1' at line %2 column %3 of song.xml")
			.arg(errorMsg).arg(errorLine).arg(errorColumn));
	}

	try
	{
		SongLoadContext ctx;
		ctx.m_doc = doc;

		QDomElement root = doc.documentElement();

//		m_routing = new Routing;
		m_routing->load(ctx, getUniqueChildElement(root, "routing"));
		m_sequence->load(ctx, getUniqueChildElement(root, "sequence"));
	}
	catch (const std::exception& err)
	{
		throw SongLoadError(QString("%1: %2").arg(typeid(err).name()).arg(err.what()));
	}
}

/////////////////////////////////////////////////////////////////////////////////////

static Ptr<Machine> loadMachine(SongLoadContext& ctx, const QDomElement& el)
{
	Ptr<Machine> mac;
	QString id = getAttribute<QString>(el, "typeid");
	try
	{
		mac = MachineFactory::get(id)->createMachine();
	}
	catch (const MachineFactory::BadIdError&)
	{
		ctx.warn(Song::tr("Machine '%1' is not installed, so it was replaced by a dummy machine.").arg(id));
		mac = new Builtin::Dummy;
	}

	mac->load(ctx, el);
	return mac;
}

void Machine::load(SongLoadContext& ctx, const QDomElement& el)
{
	m_objectUuid = getAttribute<Uuid>(el, "uuid"); ctx.setObject(m_objectUuid, this);

	setName(getAttribute<QString>(el, "name"));
	setPos(getAttribute<QPointF>(el, "pos"));
	m_halfsize = getAttribute<QPointF>(el, "hsize");

	QDomNodeList paramnodes = getUniqueChildElement(el, "parametergroup").elementsByTagName("parameter");
	for (int i=0; i<paramnodes.size(); i++)
	{
		QDomElement paramel = paramnodes.item(i).toElement();
		ParamTag tag = getAttribute<ParamTag>(paramel, "tag");
		Ptr<Parameter::Base> param = m_paramMap.value(tag, NULL);
		if (param)
			param->load(ctx, paramel);
		else
			ctx.warn(tr("Machine '%1' has no parameter with tag %2").arg(getName()).arg(tag));
	}

	loadPins(ctx, Pin::in,  getChildElements(el, "inpin"));
	loadPins(ctx, Pin::out, getChildElements(el, "outpin"));

	foreach(const QDomElement& paramPinEl, getChildElements(el, "parampin"))
	{
		Parameter::Scalar* param = ctx.getObjectOrThrow<Parameter::Scalar>(getAttribute<Uuid>(paramPinEl, "param"));
		Ptr<ParamPin> ppin = new ParamPin(
			param,
			getAttribute<TimeUnit::e>(paramPinEl, "timeunit", TimeUnit::none)
		);
		addPin(ppin);
		param->setParamPin(ppin);
		ppin->load(ctx, paramPinEl);
	}

	foreach(const QDomElement& patternEl, getChildElements(el, "pattern"))
	{
		double length = getAttribute<double>(patternEl, "length");
		Ptr<Sequence::Pattern> pattern = createPattern(length);
		pattern->load(ctx, patternEl);
		addPattern(pattern);
	}
}

/////////////////////////////////////////////////////////////////////////////////////

void Routing::load(SongLoadContext& ctx, const QDomElement& el)
{
	m_objectUuid = getAttribute<Uuid>(el, "uuid"); ctx.setObject(m_objectUuid, this);

	foreach(const QDomElement& macel, getChildElements(el, "machine"))
	{
		Ptr<Machine> mac = loadMachine(ctx, macel);
		addMachine(mac);
	}

	foreach(const QDomElement& connel, getChildElements(el, "connection"))
	{
		Ptr<Connection> conn = new Connection(
			ctx.getObjectOrThrow<Pin>(getAttribute<Uuid>(connel, "from")),
			ctx.getObjectOrThrow<Pin>(getAttribute<Uuid>(connel, "to")),
			getAttribute<bool>(connel, "feedback", false)
		);
		addConnection(conn);
	}
}

////////////////////////////////////////////////////////////////////////////////////

void Parameter::Base::load(SongLoadContext& ctx, const QDomElement& el)
{
	m_objectUuid = getAttribute<Uuid>(el, "uuid"); ctx.setObject(m_objectUuid, this);
}

void Parameter::Scalar::load(SongLoadContext& ctx, const QDomElement& el)
{
	Base::load(ctx, el);
	change(getAttribute<double>(el, "state"));
}

void Parameter::Time::load(SongLoadContext& ctx, const QDomElement& el)
{
	Scalar::load(ctx, el);
	setDisplayUnit(getAttribute<TimeUnit::e>(el, "displayunit"));
}

/////////////////////////////////////////////////////////////////////////////////////

void Machine::loadPins(SongLoadContext& ctx, Pin::Direction direction, const QList<QDomElement>& els)
{
	std::vector< Ptr<Pin> >& pins = (direction == Pin::in) ? m_inpins : m_outpins;

	for (int i=0; i<els.length(); ++i)
	{
		if (i >= pins.size())
			throw SongLoadError(tr("Machine '%1' does not have enough %2 pins")
				.arg(getName()).arg((direction == Pin::in) ? tr("input") : tr("output")) );

		pins[i]->load(ctx, els.at(i));
	}
}

void Pin::load(SongLoadContext& ctx, const QDomElement& el)
{
	m_objectUuid = getAttribute<Uuid>(el, "uuid"); ctx.setObject(m_objectUuid, this);

	setSide(getAttribute<Side>(el, "side"));
	setPos(getAttribute<float>(el, "pos"));

	if (m_type != getAttribute<SignalType::e>(el, "type"))
	{
		throw SongLoadError("Pin signal type mismatch");
	}
}

/////////////////////////////////////////////////////////////////////////////////////

void Sequence::Seq::load(SongLoadContext& ctx, const QDomElement& el)
{
	m_objectUuid = getAttribute<Uuid>(el, "uuid"); ctx.setObject(m_objectUuid, this);

	m_masterTrack.clear();
	foreach(const QDomElement& mtcel, getChildElements(el, "masterclip"))
	{
		Ptr<MasterTrackClip> mtc = new MasterTrackClip;
		mtc->load(ctx, mtcel);
		addMasterTrackClip(mtc);
	}

	foreach(const QDomElement& tel, getChildElements(el, "track"))
	{
		Ptr<Machine> machine = ctx.getObjectOrThrow<Machine>(getAttribute<Uuid>(tel, "machine"));
		Ptr<Track> track = new Track(machine);
		appendTrack(track);
		track->load(ctx, tel);
	}
}

void Sequence::Track::load(SongLoadContext& ctx, const QDomElement& el)
{
	m_objectUuid = getAttribute<Uuid>(el, "uuid"); ctx.setObject(m_objectUuid, this);

	foreach(const QDomElement& cel, getChildElements(el, "clip"))
	{
		double starttime = getAttribute<double>(cel, "starttime");
		Ptr<Pattern> pattern = ctx.getObjectOrThrow<Pattern>(getAttribute<Uuid>(cel, "pattern"));
		Ptr<Clip> clip = new Clip(starttime, pattern);
		clip->m_begin = getAttribute<double>(cel, "begin");
		clip->m_end = getAttribute<double>(cel, "end");
		addClip(clip);
	}
}

void Sequence::MasterTrackClip::load(SongLoadContext& ctx, const QDomElement& el)
{
	m_firstBeat = getAttribute<int>(el, "start");
	m_lengthInBeats = getAttribute<int>(el, "length");

	m_timeinfo.beatsPerSecond			= getAttribute<double>(el, "beatspersecond");
	m_timeinfo.beatsPerBar				= getAttribute<int>(el, "beatsperbar");
	m_timeinfo.beatsPerWholeNote		= getAttribute<int>(el, "beatsperwholenote");
	m_timeinfo.barsPerSmallGrid			= getAttribute<int>(el, "smallgridstep");
	m_timeinfo.smallGridsPerLargeGrid	= getAttribute<int>(el, "largegridstep");
}

/////////////////////////////////////////////////////////////////////////////////////

void Sequence::Pattern::load(SongLoadContext& ctx, const QDomElement& el)
{
	m_objectUuid = getAttribute<Uuid>(el, "uuid"); ctx.setObject(m_objectUuid, this);

	m_name = getAttribute<QString>(el, "name");
	m_color = getAttribute<QColor>(el, "color");
}

void Spattern::Pattern::load(SongLoadContext& ctx, const QDomElement& el)
{
	Sequence::Pattern::load(ctx, el);

	foreach(const QDomElement& nel, getChildElements(el, "note"))
	{
		Ptr<Note> note = new Note(this,
			getAttribute<double>(nel, "start"),
			getAttribute<double>(nel, "length"),
			getAttribute<double>(nel, "note")
		);
		addNote(note);
	}
}
