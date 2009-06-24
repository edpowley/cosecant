#pragma once

#include "undoredo.h"

class PatternEditor;
class NotebookWindow;
class SequenceEditor;
class Machine;
class SongLoadContext;

namespace Sequence
{
	class Pattern : public ObjectWithUuid
	{
		Q_OBJECT

	public:
		Pattern(Machine* mac);

		QString m_name;
		void setName(const QString& name);

		QColor m_color;
		void setColor(const QColor& color);

		virtual double getLength() = 0;

		void showEditor(NotebookWindow* win);
		void onEditorClose(PatternEditor* editor);

		virtual void load(SongLoadContext& ctx, xmlpp::Element* el) = 0;
		virtual void save(xmlpp::Element* el) = 0;

		virtual Ptr<Undoable> createUndoableForLengthChange(double newlength) = 0;

		Machine* m_mac;
		PatternEditor* m_editor;

	signals:
		void signalAdd();
		void signalRemove();
		void signalRename();
		void signalChangeColor();

	public:
		void added() { signalAdd(); }
		void removed() { signalRemove(); }
	};

	class Event : public Object
	{
	public:
		Event(double starttime, const Ptr<Pattern>& pattern);
		Event(const Event& other);
		double m_startTime, m_begin, m_end;
		double getLength() const { return m_end - m_begin; }
		double getEndTime() const { return m_startTime + m_end - m_begin; }
		bool spansTime(double t) { return m_startTime <= t && getEndTime() > t; }

		Ptr<Pattern> m_pattern;

		struct StartTimeLessComparer
		{
			bool operator()(const Ptr<Event> a, const Ptr<Event> b) const { return a->m_startTime < b->m_startTime; }
		};

		Event(class SongLoadContext& ctx, xmlpp::Element* el);
		void save(xmlpp::Element* el);

		static Ptr<Event> dummy(double time) { return new Event(time); }

	protected:
		Event(double starttime);
	};

	class Track : public ObjectWithUuid
	{
		Q_OBJECT

	public:
		Track(Machine* mac);
		Ptr<Machine> m_mac;

		typedef std::set<Ptr<Event>, Event::StartTimeLessComparer> Events;
		Events m_events;

		Track(class SongLoadContext& ctx, xmlpp::Element* el);
		void save(xmlpp::Element* el);

		Ptr<Event> getEventAtTime(double t);
		Ptr<Event> getNextEvent(double t);

	signals:
		void signalChange();

	public:
		void trigger_signalChange() { signalChange(); }

	protected:
		Track();
		void ctorCommon();

	protected slots:
		void onChange();
	};

	class Seq : public ObjectWithUuid
	{
		Q_OBJECT

		friend SequenceEditor;
		friend Track;
		friend Pattern;

	public:
		Seq();

		boost::shared_mutex m_mutex;

		ERROR_CLASS(TrackNotFound);

		void appendTrack(const Ptr<Track>& track);
		void insertTrack(size_t index, const Ptr<Track>& track);
		size_t getTrackIndex(const Ptr<Track>& track);
		void removeTrack(const Ptr<Track>& track);
		void removeTrack(size_t index);

		std::vector< Ptr<Sequence::Track> > m_tracks;

		Seq(class SongLoadContext& ctx, xmlpp::Element* el);
		void save(xmlpp::Element* el);

		boost::shared_mutex m_playPosMutex;
		double m_playPos, m_loopStart, m_loopEnd;
		bool m_playing;
		double m_ticksPerFrame;

		void showEditor(NotebookWindow* win);

	signals:
		void signalChange();
		void signalTracksChange();
		void signalTrackAdd(size_t idx, const Ptr<Track>& track);
		void signalTrackRemove(size_t idx, const Ptr<Track>& track);
		void signalMachinePatternsChange(const Ptr<Machine>& mac);

	public:
		void trigger_signalMachinePatternsChange(const Ptr<Machine>& mac)
		{ signalMachinePatternsChange(mac); }

	protected:
		void ctorCommon();
		SequenceEditor* m_editor;
	};
};
