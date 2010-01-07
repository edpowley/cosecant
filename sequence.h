#pragma once

namespace Seq
{
	class Track; class Clip;

	class Sequence : public Object
	{
	protected:
		QList<Track> m_tracks;
	};

	class Track : public Object
	{
	protected:
		QMap<int64, Ptr<Clip> > m_clips;
	};

	class Clip : public Object
	{
	protected:
		int64 m_length;
	};

};

using Seq::Sequence;
