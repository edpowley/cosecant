#include "stdafx.h"
#include "common.h"
#include "sequence.h"
using namespace Sequence;
#include "machine.h"
//#include "notebookwindow.h"
#include "song.h"
//#include "sequenceeditor.h"
//#include "patterneditor.h"

Seq::Seq()
{
	ctorCommon();
}

void Seq::ctorCommon()
{
	m_editor = NULL;

	m_playPos = 0;
	m_playing = false;
	m_loopStart = 0;
	m_loopEnd = 16;
	// tick/sample = (tick/minute) / (second/minute) / (sample/second)
	m_ticksPerFrame = 120.0 / 60.0 / 44100.0;
}

////////////////////////////////////////////////////////////////////////////

Clip::Clip(double starttime, const Ptr<Sequence::Pattern>& pattern)
: m_startTime(starttime), m_pattern(pattern)
{
	m_begin = 0.0;
	m_end = m_pattern->getLength();
}

Clip::Clip(const Clip& other)
: m_startTime(other.m_startTime), m_begin(other.m_begin), m_end(other.m_end), m_pattern(other.m_pattern)
{
}

Clip::Clip(double starttime)
: m_startTime(starttime)
{
	m_begin = m_end = 0.0;
}

////////////////////////////////////////////////////////////////////////////

Track::Track()
: m_mac(NULL)
{
	ctorCommon();
}

Track::Track(Machine* mac)
: m_mac(mac)
{
	ctorCommon();
}

void Track::ctorCommon()
{
	connect(
		this, SIGNAL(signalChange()),
		this, SLOT(onChange())
	);
}

void Track::onChange()
{
	// TODO: change this
	if (Song::get().m_sequence)
		Song::get().m_sequence->signalTracksChange();
}

Ptr<Clip> Track::getClipAtTime(double t)
{
	Clips::iterator i = m_clips.upper_bound(Clip::dummy(t)); // First with starttime > t
	if (i == m_clips.begin()) return NULL;

	--i;
	if ((*i)->spansTime(t))
		return *i;
	else
		return NULL;
}

Ptr<Clip> Track::getNextClip(double t)
{
	Clips::iterator i = m_clips.upper_bound(Clip::dummy(t)); // First with starttime > t
	if (i != m_clips.end())
		return *i;
	else
		return NULL;
}

////////////////////////////////////////////////////////////////////////////

void Seq::appendTrack(const Ptr<Track>& track)
{
	m_tracks.push_back(track);
	signalTrackAdd(m_tracks.size()-1, track);
	signalTracksChange();
}

void Seq::insertTrack(size_t index, const Ptr<Track>& track)
{
	m_tracks.insert(m_tracks.begin() + index, track);
	signalTrackAdd(index, track);
	signalTracksChange();
}

size_t Seq::getTrackIndex(const Ptr<Track>& track)
{
	for (size_t i=0; i<m_tracks.size(); i++)
	{
		if (m_tracks[i] == track)
			return i;
	}

	throw TrackNotFound();
}

void Seq::removeTrack(size_t index)
{
	Ptr<Track> track = m_tracks[index];
	m_tracks.erase(m_tracks.begin() + index);
	signalTrackRemove(index, track);
	signalTracksChange();
}

void Seq::removeTrack(const Ptr<Track>& track)
{
	removeTrack(getTrackIndex(track));
}

////////////////////////////////////////////////////////////////////////////

void Seq::showEditor(NotebookWindow* win)
{
/*	if (!m_editor)
	{
		m_editor = Gtk::manage(new SequenceEditor(this));
		win->addPage(m_editor);
	}
	NotebookWindow::presentPage(m_editor);
*/
}

////////////////////////////////////////////////////////////////////////////////

Pattern::Pattern(Machine* mac)
: m_mac(mac), m_editor(NULL), m_name("Unnamed"), m_color("#80FF80")
{
}

void Pattern::showEditor(NotebookWindow* win)
{
/*	if (!m_editor)
	{
		m_editor = Gtk::manage(m_mac->createPatternEditor(this));
		win->addPage(m_editor);
	}
	NotebookWindow::presentPage(m_editor);
*/
}

void Pattern::onEditorClose(PatternEditor* editor)
{
	if (m_editor == editor)
		m_editor = NULL;
}

void Pattern::setName(const QString& name)
{
	if (m_name != name)
	{
		m_name = name;
		signalRename();
		emit Song::get().m_sequence->signalChange();
	}
}

void Pattern::setColor(const QColor& color)
{
	if (m_color != color)
	{
		m_color = color;
		signalChangeColor();
		Song::get().m_sequence->signalChange();
	}
}
