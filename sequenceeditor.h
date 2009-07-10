#pragma once

#include "sequence.h"

namespace SequenceEditor
{
	class Editor : public QSplitter
	{
		Q_OBJECT

	public:
		Editor(const Ptr<Sequence::Seq>& seq, QWidget* parent);

	protected:
		Ptr<Sequence::Seq> m_seq;

		QGraphicsView *m_headView, *m_bodyView, *m_rulerView;

		QGraphicsScene m_headScene, m_bodyScene, m_rulerScene;
	};
};
