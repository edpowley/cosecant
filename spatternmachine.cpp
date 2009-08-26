#include "stdafx.h"
#include "common.h"
#include "spatternmachine.h"
#include "spattern.h"
#include "spatterneditor.h"
#include "seqplay.h"

SpatternMachine::SpatternMachine()
{
}

QDebug& operator<<(QDebug& dbg, const StreamEvent& ev)
{
	dbg.space() << "StreamEvent(" << ev.time;
	switch (ev.type)
	{
	case StreamEventType::noteOn:
		dbg << "note on" << ev.note.id << ev.note.note << ev.note.vel;
		break;
	
	case StreamEventType::noteOff:
		dbg << "note off" << ev.note.id;
		break;
	
	default:
		dbg << "unknown type" << ev.type;
		break;
	}
	dbg << ")";

	return dbg;
}

void SpatternMachine::work(const WorkContext* ctx)
{
	Ptr<WorkBuffer::Events> outbuf = dynamic_cast<WorkBuffer::Events*>(ctx->out[0].hostbuf);
	const TimeInfo& timeinfo = SeqPlay::get().getTimeInfo();
	double beatsPerFrame = timeinfo.beatsPerSecond / timeinfo.samplesPerSecond;

	foreach(const Ptr<Spattern::Note>& note, m_stoppingNotes)
	{
		StreamEvent ev;
		ev.time = ctx->firstframe;
		ev.type = StreamEventType::noteOff;
		ev.note.id = note.c_ptr();
		outbuf->m_data.insert(ev);
	}
	m_stoppingNotes.clear();

	foreach(const Ptr<SpatternPlayer>& player, m_players)
	{
		player->work(outbuf, ctx->firstframe, ctx->lastframe, beatsPerFrame);
	}
}

void SpatternMachine::playPattern(Sequence::Track* track, Spattern::Pattern* patt, double startpos)
{
	Ptr<SpatternPlayer> player = new SpatternPlayer(patt, startpos);
	m_players.insert(track, player);
}

void SpatternMachine::stopPattern(Sequence::Track* track, Spattern::Pattern* patt)
{
	SpatternPlayer* player = m_players.value(track, NULL);
	if (player && player->getPattern().c_ptr() == patt)
	{
		m_stoppingNotes.append(player->getPlayingNotes());
		m_players.remove(track);
	}
}

void SpatternMachine::initInfo()
{
	static MachineInfo info;
	static bool initialised = false;
	if (!initialised)
	{
		info.defaultName = "Standard Pattern";
		info.typeHint = MachineTypeHint::control;
		info.flags = MachineFlags::hasCustomPatterns | MachineFlags::createSequenceTrack;

		static PinInfo pinNoteOut;
		pinNoteOut.name = "Note out";
		pinNoteOut.type = SignalType::noteTrigger;

		static const PinInfo* outPins[] = {&pinNoteOut, NULL};
		info.outPins = outPins;

		initialised = true;
	}

	m_info = &info;
}

void SpatternMachine::initImpl()
{
}

Ptr<Sequence::Pattern> SpatternMachine::createPatternImpl(double length)
{
	return new Spattern::Pattern(this, length);
}

QWidget* SpatternMachine::createPatternEditorWidget(const Ptr<Sequence::Pattern>& pattern)
{
	Spattern::Pattern* spatt = dynamic_cast<Spattern::Pattern*>(pattern.c_ptr());
	return new SpatternEditor::Editor(spatt);
}

////////////////////////////////////////////////////////////////////////////

SpatternPlayer::SpatternPlayer(const Ptr<Spattern::Pattern>& pattern, double startpos)
: m_pattern(pattern), m_pos(startpos)
{
	resetIter();
}

void SpatternPlayer::resetIter()
{
	m_iter = m_pattern->m_notes.lowerBound(m_pos); // m_iter is first with key >= m_pos
	m_enditer = m_pattern->m_notes.end();
}

void SpatternPlayer::work(const Ptr<WorkBuffer::Events>& outbuf,
						  int firstframe, int lastframe, double beatsPerFrame)
{

	double nextpos = m_pos + beatsPerFrame * (lastframe - firstframe);
	for(; m_iter != m_enditer && m_iter.key() < nextpos; ++m_iter)
	{
		if (m_iter.key() < m_pos) continue;

		StreamEvent ev;
		ev.time = (int)floor(firstframe + (m_iter.key() - m_pos) / beatsPerFrame);
		ev.type = StreamEventType::noteOn;
		ev.note.id = m_iter.value().c_ptr();
		ev.note.note = m_iter.value()->getNote();
		ev.note.vel = 1.0;

		outbuf->m_data.insert(ev);

		m_playingNotes.append(m_iter.value());
	}

	for(QList< Ptr<Spattern::Note> >::iterator iter = m_playingNotes.begin(); iter != m_playingNotes.end(); )
	{
		const Ptr<Spattern::Note>& note = *iter;
		if (note->getEnd() < nextpos)
		{
			StreamEvent ev;
			ev.time = (int)floor(firstframe + (note->getEnd() - m_pos) / beatsPerFrame);
			ev.time = max(ev.time, firstframe);
			ev.type = StreamEventType::noteOff;
			ev.note.id = note.c_ptr();

			outbuf->m_data.insert(ev);

			iter = m_playingNotes.erase(iter);
		}
		else
		{
			++iter;
		}
	}

	m_pos = nextpos;
}
