#pragma once

class PatternEditor;
class NotebookWindow;
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

//		virtual void load(SongLoadContext& ctx, const QDomElement& el) = 0;
//		virtual void save(const QDomElement& el) = 0;

		virtual QUndoCommand* createUndoableForLengthChange(double newlength) = 0;

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

	class Clip : public Object
	{
	public:
		Clip(double starttime, const Ptr<Pattern>& pattern);
		Clip(const Clip& other);
		double m_startTime, m_begin, m_end;
		double getLength() const { return m_end - m_begin; }
		double getEndTime() const { return m_startTime + m_end - m_begin; }
		bool spansTime(double t) { return m_startTime <= t && getEndTime() > t; }

		Ptr<Pattern> m_pattern;

		struct StartTimeLessComparer
		{
			bool operator()(const Ptr<Clip> a, const Ptr<Clip> b) const { return a->m_startTime < b->m_startTime; }
		};

		Clip(class SongLoadContext& ctx, const QDomElement& el);
		void save(const QDomElement& el);

		static Ptr<Clip> dummy(double time) { return new Clip(time); }

	protected:
		Clip(double starttime);
	};

	class Track : public ObjectWithUuid
	{
		Q_OBJECT

	public:
		Track(Machine* mac);
		Ptr<Machine> m_mac;

		typedef std::set<Ptr<Clip>, Clip::StartTimeLessComparer> Clips;
		Clips m_clips;

		Track(class SongLoadContext& ctx, const QDomElement& el);
		void save(const QDomElement& el);

		Ptr<Clip> getClipAtTime(double t);
		Ptr<Clip> getNextClip(double t);

		double getHeight() { return m_height; }

	protected:
		Track();
		void ctorCommon();

		double m_height;
	};

	class MasterTrackClip : public Object
	{
		Q_OBJECT

	public:
		MasterTrackClip() : m_bpm(120), m_tpb(4), m_bpb(4), m_gridStep(4), m_lengthInTicks(-1) {}
		
		void setBPM(double bpm) { m_bpm = bpm; }
		void setTPB(int tpb) { m_tpb = tpb; }
		void setBPB(int bpb) { m_bpb = bpb; } // BPB = beats per bar
		void setGridStep(int gs) { m_gridStep = gs; }

		double getBPM() { return m_bpm; }
		int getTPB() { return m_tpb; }
		int getBPB() { return m_bpb; }
		int getGridStep() { return m_gridStep; }

		double getTicksPerSecond() { return m_bpm * m_tpb / 60.0; }

	protected:
		double m_bpm;
		int m_tpb, m_bpb, m_gridStep;
		
		int m_lengthInTicks;
	};

	class Seq : public ObjectWithUuid
	{
		Q_OBJECT

		friend Track;
		friend Pattern;

	public:
		Seq();

		boost::shared_mutex m_mutex;

		ERROR_CLASS(TrackNotFound);

		void appendTrack(const Ptr<Track>& track);
		void insertTrack(int index, const Ptr<Track>& track);
		int getTrackIndex(const Ptr<Track>& track);
		void removeTrack(const Ptr<Track>& track);
		void removeTrack(int index);

		typedef QMap< int, Ptr<Track> > TrackIndexMap;
		TrackIndexMap getTracksForMachine(const Ptr<Machine>& mac);
		void insertTracks(const TrackIndexMap& tim);
		void removeTracks(const TrackIndexMap& tim);

		QList< Ptr<Sequence::Track> > m_tracks;
		QMap<double, Ptr<MasterTrackClip> > m_masterTrack;

		Seq(class SongLoadContext& ctx, const QDomElement& el);
		void save(const QDomElement& el);

		boost::shared_mutex m_playPosMutex;
		double m_playPos, m_loopStart, m_loopEnd;
		bool m_playing;
		double m_ticksPerFrame;

		void showEditor(NotebookWindow* win);

		double getLengthInSeconds() { return 400.0; }

	signals:
		// Have to specify Sequence::Track (instead of just Track) here, as Qt isn't smart enough to figure
		// out they're the same thing
		void signalInsertTrack(int index, const Ptr<Sequence::Track>& track);
		void signalRemoveTrack(int index, const Ptr<Sequence::Track>& track);

	protected:
		void ctorCommon();
	};
};
