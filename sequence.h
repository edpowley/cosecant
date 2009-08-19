#pragma once

#include "cosecant_api.h"

class PatternEditor;
class NotebookWindow;
class Machine;
class SongLoadContext;

namespace Sequence
{
	class Track;

	class Pattern : public ObjectWithUuid
	{
		friend class PatternEditor;

		Q_OBJECT

	public:
		Pattern(Machine* mac, double length);
		virtual ~Pattern();

		QString m_name;
		void setName(const QString& name);

		QColor m_color;
		void setColor(const QColor& color);

		double getLength() { return m_length; }

		void showEditor();

		Machine* m_mac;

		virtual void play(Track* track, double startpos) = 0;
		virtual void stop(Track* track) = 0;

		virtual void load(SongLoadContext& ctx, const QDomElement& el);
		virtual QDomElement save(QDomDocument& doc);

	signals:
		void signalAdd();
		void signalRemove();
		void signalRename();
		void signalChangeColor();

	public:
		void added() { signalAdd(); }
		void removed() { signalRemove(); }

	protected:
		CosecantAPI::MiPattern* m_miPattern;
		PatternEditor* m_editor;
		double m_length;
	};

	//////////////////////////////////////////////////////////////////////////////////

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

		void load(SongLoadContext& ctx, const QDomElement& el);
		QDomElement save(QDomDocument& doc);
	};

	/////////////////////////////////////////////////////////////////////////////

	class Track : public ObjectWithUuid
	{
		Q_OBJECT

	public:
		Track(Machine* mac);
		Ptr<Machine> m_mac;

		typedef QMap<double, Ptr<Clip> > Clips;
		void addClip(const Ptr<Clip>& clip);
		void removeClip(const Ptr<Clip>& clip);

		const Clips& getClips() { return m_clips; }

		double getHeight() { return m_height; }

		void load(SongLoadContext& ctx, const QDomElement& el);
		QDomElement save(QDomDocument& doc);

	signals:
		void signalAddClip(const Ptr<Sequence::Clip>& clip);
		void signalRemoveClip(const Ptr<Sequence::Clip>& clip);

	protected:
		Track();
		void ctorCommon();

		double m_height;

		Clips m_clips;
	};

	/////////////////////////////////////////////////////////////////////////////

	class MasterTrackClip : public Object
	{
		Q_OBJECT

	public:
		MasterTrackClip();
		
		CosecantAPI::TimeInfo getTimeInfo() { return m_timeinfo; }

		int getFirstBeat() { return m_firstBeat; }
		void setFirstBeat(int fb) { m_firstBeat = fb; }

	protected:
		CosecantAPI::TimeInfo m_timeinfo;		
		int m_firstBeat, m_lengthInBeats;
	};

	//////////////////////////////////////////////////////////////////////////////

	class Seq : public ObjectWithUuid
	{
		Q_OBJECT

		friend Track;
		friend Pattern;

	public:
		Seq();

		// It is VERY IMPORTANT that m_mutex is recursive, because SeqPlay::work WILL lock it recursively
		QReadWriteLock_Recursive m_mutex;

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
		QMap<int, Ptr<MasterTrackClip> > m_masterTrack; // key = start time in beats

		double m_loopStart, m_loopEnd;

		void showEditor(NotebookWindow* win);

		double getLengthInSeconds() { return 400.0; }
		double beatToSecond(double b);
		double secondToBeat(double s);

		void load(SongLoadContext& ctx, const QDomElement& el);
		QDomElement save(QDomDocument& doc);

	signals:
		// Have to specify Sequence::Track (instead of just Track) here, as Qt isn't smart enough to figure
		// out they're the same thing
		void signalInsertTrack(int index, const Ptr<Sequence::Track>& track);
		void signalRemoveTrack(int index, const Ptr<Sequence::Track>& track);

	protected:
		void ctorCommon();

		QHash<Ptr<MasterTrackClip>, double> m_mtcStartTimes; // value = start time in seconds
		QMap<double, Ptr<MasterTrackClip> > m_mtcStartTimes2;  // key = start time in seconds
	};
};
