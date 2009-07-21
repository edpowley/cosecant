#include "stdafx.h"
#include "common.h"
#include "sequence.h"
using namespace Sequence;
#include "machine.h"
#include "song.h"

Seq::Seq()
{
	ctorCommon();
}

void Seq::ctorCommon()
{
	m_loopStart = 0;
	m_loopEnd = 32;

	Ptr<MasterTrackClip> mtc = new MasterTrackClip;
	m_masterTrack.insert(0, mtc);
	m_mtcStartTimes.insert(mtc, 0);
}

double Seq::beatToSecond(double b)
{
	QMap<int, Ptr<MasterTrackClip> >::const_iterator iter = m_masterTrack.upperBound((int)floor(b));
	// Now iter points to the first item with key > b

	-- iter;
	// Now iter points to the last item with key <= b

	double posInMtc = b - iter.key();
	const Ptr<MasterTrackClip>& mtc = iter.value();
	return m_mtcStartTimes.value(mtc, 0) + posInMtc / mtc->getTimeInfo().beatsPerSecond;
}

////////////////////////////////////////////////////////////////////////////

MasterTrackClip::MasterTrackClip()
{
	m_timeinfo.beatsPerSecond = 120.0 / 60.0;
	m_timeinfo.beatsPerBar = 4;
	m_timeinfo.beatsPerWholeNote = 4;
	m_timeinfo.barsPerSmallGrid = 4;
	m_timeinfo.smallGridsPerLargeGrid = 4;
	m_timeinfo.samplesPerSecond = -1;

	m_lengthInTicks = -1;
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
	m_height = 100;
}

Ptr<Clip> Track::getClipAtTime(double t)
{
	Clips::iterator i = m_clips.upperBound(t); // First with starttime > t
	if (i == m_clips.begin()) return NULL;

	--i;
	if ((*i)->spansTime(t))
		return *i;
	else
		return NULL;
}

Ptr<Clip> Track::getNextClip(double t)
{
	Clips::iterator i = m_clips.upperBound(t); // First with starttime > t
	if (i != m_clips.end())
		return *i;
	else
		return NULL;
}

////////////////////////////////////////////////////////////////////////////

void Seq::appendTrack(const Ptr<Track>& track)
{
	m_tracks.push_back(track);
	signalInsertTrack(m_tracks.length()-1, track);
}

void Seq::insertTrack(int index, const Ptr<Track>& track)
{
	m_tracks.insert(index, track);
	signalInsertTrack(index, track);
}

int Seq::getTrackIndex(const Ptr<Track>& track)
{
	int i = m_tracks.indexOf(track);
	if (i != -1)
		return i;
	else
		throw TrackNotFound();
}

void Seq::removeTrack(int index)
{
	Ptr<Track> track = m_tracks[index];
	m_tracks.removeAt(index);
	signalRemoveTrack(index, track);
}

void Seq::removeTrack(const Ptr<Track>& track)
{
	int index = getTrackIndex(track);
	removeTrack(index);
	signalRemoveTrack(index, track);
}

Seq::TrackIndexMap Seq::getTracksForMachine(const Ptr<Machine>& mac)
{
	TrackIndexMap ret;
	
	for (int i=0; i<m_tracks.length(); i++)
	{
		if (m_tracks[i]->m_mac == mac)
			ret.insert(i, m_tracks[i]);
	}

	return ret;
}

void Seq::removeTracks(const TrackIndexMap& tim)
{
	QMapIterator< int, Ptr<Track> > iter(tim); iter.toBack();
	while (iter.hasPrevious())
	{
		iter.previous();
		ASSERT(iter.value() == m_tracks[iter.key()]);
		m_tracks.removeAt(iter.key());
		signalRemoveTrack(iter.key(), iter.value());
	}
}

void Seq::insertTracks(const TrackIndexMap& tim)
{
	QMapIterator< int, Ptr<Track> > iter(tim);
	while (iter.hasNext())
	{
		iter.next();
		m_tracks.insert(iter.key(), iter.value());
		signalInsertTrack(iter.key(), iter.value());
	}
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
	}
}

void Pattern::setColor(const QColor& color)
{
	if (m_color != color)
	{
		m_color = color;
		signalChangeColor();
	}
}
