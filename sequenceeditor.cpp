#include "stdafx.h"
#include "common.h"
#include "sequenceeditor.h"
using namespace SequenceEditor;
#include "song.h"
#include "seqplay.h"
#include "theme.h"
#include "machine.h"
#include "application.h"
#include "patterneditor.h"

/* TRANSLATOR SequenceEditor::Editor */

enum ZValues
{
	zRulerBar,
	zLoopMarker,
	zRulerLabel,
	zRulerBpmText,
	zPlayLine,
};

Editor::Editor(const Ptr<Sequence::Seq>& seq, QWidget* parent)
: QSplitter(parent), m_seq(seq)
{
	setupToolBar();

	QWidget* whead = new QWidget(this);
	QWidget* wbody = new QWidget(this);
	addWidget(whead); addWidget(wbody);
	setStretchFactor(0,0); setStretchFactor(1,1);

	m_headView = new MyGraphicsView(this);
	m_rulerView = new MyGraphicsView(this);
	m_bodyView = new MyGraphicsView(this);

	QVBoxLayout* lhead = new QVBoxLayout(whead);
	lhead->setContentsMargins(0,0,0,0);
	lhead->addSpacing(c_rulerHeight + lhead->spacing());
	lhead->addWidget(m_headView, 1);
	
	QVBoxLayout* lbody = new QVBoxLayout(wbody);
	lbody->setContentsMargins(0,0,0,0);
	m_rulerView->setMinimumHeight(c_rulerHeight);
	m_rulerView->setMaximumHeight(c_rulerHeight);
	lbody->addWidget(m_rulerView, 0);
	lbody->addWidget(m_bodyView, 1);

	m_headView ->setHorizontalScrollBarPolicy	(Qt::ScrollBarAlwaysOff);
	m_headView ->setVerticalScrollBarPolicy		(Qt::ScrollBarAlwaysOff);
	m_rulerView->setHorizontalScrollBarPolicy	(Qt::ScrollBarAlwaysOff);
	m_rulerView->setVerticalScrollBarPolicy		(Qt::ScrollBarAlwaysOff);
	m_bodyView ->setHorizontalScrollBarPolicy	(Qt::ScrollBarAlwaysOn);
	m_bodyView ->setVerticalScrollBarPolicy		(Qt::ScrollBarAlwaysOn);

	connect(
		m_bodyView->horizontalScrollBar(), SIGNAL(valueChanged(int)),
		m_rulerView->horizontalScrollBar(), SLOT(setValue(int)) );

	connect(
		m_bodyView->verticalScrollBar(), SIGNAL(valueChanged(int)),
		m_headView->verticalScrollBar(), SLOT(setValue(int)) );

	m_rulerView->setScene(&m_rulerScene);
	m_headView ->setScene(&m_headScene);
	m_bodyView ->setScene(&m_bodyScene);
	m_rulerView->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	m_headView ->setAlignment(Qt::AlignLeft | Qt::AlignTop);
	m_bodyView ->setAlignment(Qt::AlignLeft | Qt::AlignTop);

	setZoom(16);

	m_rulerScene.setSceneRect(0, 0, m_seq->getLengthInSeconds(), c_rulerHeight);
	m_bodyScene .setSceneRect(0, 0, m_seq->getLengthInSeconds(), 1000);

	createTrackItems();
	createRulerSectionItems();

	connect(
		m_seq, SIGNAL(signalInsertTrack(int, const Ptr<Sequence::Track>&)),
		this, SLOT(onInsertTrack(int, const Ptr<Sequence::Track>&)) );
	connect(
		m_seq, SIGNAL(signalRemoveTrack(int, const Ptr<Sequence::Track>&)),
		this, SLOT(onRemoveTrack(int, const Ptr<Sequence::Track>&)) );

	m_rulerPlayLine = new PlayLineItem(this, c_rulerHeight);
	m_rulerPlayLine->setZValue(zPlayLine);
	m_rulerScene.addItem(m_rulerPlayLine);

	m_bodyPlayLine = new PlayLineItem(this, 1e6);
	m_bodyPlayLine->setZValue(zPlayLine);
	m_bodyScene.addItem(m_bodyPlayLine);

	m_loopStartItem = new LoopMarkerItem(this, Editor::c_rulerHeight, LoopMarkerItem::loopStart);
	m_loopStartItem->setPos(m_seq->beatToSecond(m_seq->m_loopStart), 0);
	m_loopStartItem->setZValue(zLoopMarker);
	m_rulerScene.addItem(m_loopStartItem);

	m_loopEndItem = new LoopMarkerItem(this, Editor::c_rulerHeight, LoopMarkerItem::loopEnd);
	m_loopEndItem->setPos(m_seq->beatToSecond(m_seq->m_loopEnd), 0);
	m_loopEndItem->setZValue(zLoopMarker);
	m_rulerScene.addItem(m_loopEndItem);

	m_playPosTimer.setSingleShot(false);
	connect( &m_playPosTimer, SIGNAL(timeout()),
		this, SLOT(onPlayPosTimer()) );
	m_playPosTimer.start(1000 / 25);
}

void Editor::setupToolBar()
{
	m_toolbar = new QToolBar(tr("Sequence editor toolbar"));
	QActionGroup* toolGroup = new QActionGroup(this);

	m_actionToolMove
		= new QAction(QIcon(":/CosecantMainWindow/images/seqed-move.png"), tr("Move"), this);
	m_actionToolMove->setCheckable(true);
	m_toolbar->addAction(m_actionToolMove);
	toolGroup->addAction(m_actionToolMove);
	m_actionToolMove->setChecked(true);

	m_actionToolCreatePattern
		= new QAction(QIcon(":/CosecantMainWindow/images/seqed-create.png"), tr("Create pattern"), this);
	m_actionToolCreatePattern->setCheckable(true);
	m_toolbar->addAction(m_actionToolCreatePattern);
	toolGroup->addAction(m_actionToolCreatePattern);
}

Editor::Tool Editor::getTool()
{
	if (m_actionToolMove->isChecked())
		return move;
	else if (m_actionToolCreatePattern->isChecked())
		return createPattern;
	else
		return none;
}

void Editor::setZoom(double pixelsPerSecond)
{
	QTransform t; t.scale(pixelsPerSecond, 1);
	m_rulerView->setTransform(t);
	m_bodyView ->setTransform(t);
}

qreal Editor::getBodyWidth()
{
	return m_seq->getLengthInSeconds();
}

void Editor::createTrackItems()
{
	qreal y = 0.0;
	for (int i=0; i<m_seq->m_tracks.length(); ++i)
	{
		TrackItem* item = new TrackItem(this, m_seq->m_tracks[i]);
		m_bodyScene.addItem(item);
		item->setPos(0, y);

		m_trackItems.push_back(item);

		y += item->height();
	}
}

void Editor::onInsertTrack(int index, const Ptr<Sequence::Track>& track)
{
	qreal y = 0.0;
	if (index < m_trackItems.length())
		y = m_trackItems[index]->y();
	else if (index > 0)
		y = m_trackItems[index-1]->y() + m_trackItems[index-1]->height();

	TrackItem* item = new TrackItem(this, track);
	m_bodyScene.addItem(item);
	item->setPos(0, y);

	m_trackItems.insert(index, item);

	// Move the subsequent items down
	qreal h = item->height();
	for (int i = index+1; i < m_trackItems.length(); ++i)
	{
		m_trackItems[i]->moveBy(0, h);
	}
}

void Editor::onRemoveTrack(int index, const Ptr<Sequence::Track>& track)
{
	TrackItem* item = m_trackItems.takeAt(index); // removes it from the list
	m_bodyScene.removeItem(item);

	// Move the subsequent items up
	qreal h = item->height();
	for (int i = index; i < m_trackItems.length(); ++i)
	{
		m_trackItems[i]->moveBy(0, -h);
	}

	delete item;
}

void Editor::createRulerSectionItems()
{
	QMapIterator<int, Ptr<Sequence::MasterTrackClip> > iter(m_seq->m_masterTrack);
	while (iter.hasNext())
	{
		iter.next();
		RulerSectionItem* item = new RulerSectionItem(this, iter.value());
		item->setZValue(zRulerBar);
		m_rulerSectionItems.insert(iter.value(), item);
		m_rulerScene.addItem(item);
		item->setPos(0, m_seq->beatToSecond(iter.key()));
	}
}

void Editor::onPlayPosTimer()
{
	QReadLocker lock(&SeqPlay::get().m_mutex);

	if (SeqPlay::get().isPlaying())
	{
		signalChangePlayPos(m_seq->beatToSecond(SeqPlay::get().getPlayPos()));
	}
}

double Editor::getNearestSnapPoint(double x)
{
	RulerSectionItem* rsi = NULL;
	foreach(QGraphicsItem* i, m_rulerScene.items(QPointF(x, 0)))
	{
		rsi = dynamic_cast<RulerSectionItem*>(i);
		if (rsi != NULL) break;
	}

	if (!rsi) return 0;

	return rsi->pos().x() + rsi->getNearestSnapPoint(x - rsi->pos().x());
}

///////////////////////////////////////////////////////////////////////////////

TrackItem::TrackItem(Editor* editor, const Ptr<Sequence::Track>& track)
:	m_editor(editor), m_track(track), QGraphicsRectItem(0, 0, editor->getBodyWidth(), track->getHeight()),
	m_mouseMode(none), m_newClipItem(NULL)
{
	setPen(Qt::NoPen);
	QLinearGradient grad(0, 0, 0, rect().height());
	grad.setColorAt(0, Qt::white);
	grad.setColorAt(1, Qt::gray);
	setBrush(grad);

	connect( track, SIGNAL(signalAddClip(const Ptr<Sequence::Clip>&)),
		this, SLOT(onAddClip(const Ptr<Sequence::Clip>&)) );
	connect( track, SIGNAL(signalRemoveClip(const Ptr<Sequence::Clip>&)),
		this, SLOT(onRemoveClip(const Ptr<Sequence::Clip>&)) );
}

void TrackItem::onAddClip(const Ptr<Sequence::Clip>& clip)
{
	ClipItem* i = new ClipItem(m_editor, this, clip);
	m_clipItems.insert(clip, i);
}

void TrackItem::onRemoveClip(const Ptr<Sequence::Clip>& clip)
{
	ClipItem* i = m_clipItems.take(clip);
	if (i) delete i;
}

double TrackItem::getNearestSnapPoint(double x)
{
	return m_editor->getNearestSnapPoint(x);
}

void TrackItem::mousePressEvent(QGraphicsSceneMouseEvent* ev)
{
	if (ev->button() == Qt::LeftButton && m_editor->getTool() == Editor::createPattern)
	{
		ev->accept();
		m_mouseMode = drawNewClip;
		double x = getNearestSnapPoint(ev->pos().x());
		m_newClipItem = new QGraphicsRectItem(x, 0, 0, rect().height(), this);
		QPen pen(Qt::black, 2, Qt::DashLine);
		pen.setCosmetic(true);
		m_newClipItem->setPen(pen);
	}
}

void TrackItem::mouseMoveEvent(QGraphicsSceneMouseEvent* ev)
{
	switch (m_mouseMode)
	{
	case drawNewClip:
		{
			ev->accept();
			double x = getNearestSnapPoint(ev->pos().x());
			QRectF r = m_newClipItem->rect();
			x = max(x, r.left());
			r.setRight(x);
			m_newClipItem->setRect(r);
		}
		break;
	}
}

class AddClipCommand : public QUndoCommand
{
public:
	AddClipCommand(const Ptr<Sequence::Track>& track, const Ptr<Sequence::Clip>& clip, bool createPattern)
		: m_track(track), m_clip(clip), m_createPattern(createPattern)
	{
		if (m_createPattern)
			setText(Editor::tr("insert new pattern"));
		else
			setText(Editor::tr("insert pattern"));
	}

	virtual void redo()
	{
		if (m_createPattern) m_track->m_mac->addPattern(m_clip->m_pattern);
		m_track->addClip(m_clip);
	}

	virtual void undo()
	{
		m_track->removeClip(m_clip);
		if (m_createPattern) m_track->m_mac->removePattern(m_clip->m_pattern);
	}

protected:
	Ptr<Sequence::Track> m_track;
	Ptr<Sequence::Clip> m_clip;
	bool m_createPattern;
};

void TrackItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* ev)
{
	switch (m_mouseMode)
	{
	case drawNewClip:
		if (ev->button() == Qt::LeftButton)
		{
			ev->accept();
			double startTime = m_newClipItem->rect().left();
			double endTime   = startTime + m_newClipItem->rect().width();
			double startBeat = m_editor->getSeq()->secondToBeat(startTime);
			double endBeat   = m_editor->getSeq()->secondToBeat(endTime);

			if (endBeat > startBeat)
			{
				Ptr<Sequence::Pattern> patt = m_track->m_mac->createPattern(endBeat - startBeat);
				Ptr<Sequence::Clip> clip = new Sequence::Clip(startBeat, patt);
				
				theUndo().push(new AddClipCommand(m_track, clip, true));
			}

			delete m_newClipItem;
			m_newClipItem = NULL;
			m_mouseMode = none;
		}
		break;
	};
}

///////////////////////////////////////////////////////////////////////////////

ClipItem::ClipItem(Editor* editor, TrackItem* track, const Ptr<Sequence::Clip>& clip)
: QGraphicsRectItem(track), m_editor(editor), m_track(track), m_clip(clip)
{
	QRectF r;
	r.setWidth( m_editor->getSeq()->beatToSecond(m_clip->getLength()) );
	r.setTop(0);
	r.setHeight(track->rect().height());
	setRect(r);

	setPos( m_editor->getSeq()->beatToSecond(m_clip->m_startTime), 0 );

	setBrush(m_clip->m_pattern->m_color);
	QPen pen(Qt::black, 1);
	pen.setCosmetic(true);
	pen.setJoinStyle(Qt::MiterJoin);
	setPen(pen);
}

void ClipItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* ev)
{
	if (ev->button() == Qt::LeftButton)
	{
		m_clip->m_pattern->showEditor();
	}
}

///////////////////////////////////////////////////////////////////////////////

RulerSectionItem::RulerSectionItem(Editor* editor, const Ptr<Sequence::MasterTrackClip>& mtc)
:	m_editor(editor), m_mtc(mtc), QGraphicsRectItem(0, 0, editor->getBodyWidth(), Editor::c_rulerHeight),
	m_bpmFont(editor->font())
{
	m_bpmFont.setPixelSize(Editor::c_rulerHeight * 3 / 4);
	m_bpmFont.setBold(true);

	setPen(Qt::NoPen);

	m_colSmallGrid = Theme::get().getColor("SequenceEditor/Ruler/SmallGrid");
	m_colLargeGrid = Theme::get().getColor("SequenceEditor/Ruler/LargeGrid");
	m_colSmallGridLabel = Theme::get().getColor("SequenceEditor/Ruler/SmallGridLabel");
	m_colLargeGridLabel = Theme::get().getColor("SequenceEditor/Ruler/LargeGridLabel");

	m_colOdd  = Theme::get().getColor("SequenceEditor/Ruler/Odd");
	m_colEven = Theme::get().getColor("SequenceEditor/Ruler/Even");

	setupChildren();
}

void RulerSectionItem::setupChildren()
{
	CosecantAPI::TimeInfo ti = m_mtc->getTimeInfo();

	QGraphicsSimpleTextItem* bpmtext = new QGraphicsSimpleTextItem(this);
	bpmtext->setZValue(zRulerBpmText);
	bpmtext->setFlag(QGraphicsItem::ItemIgnoresTransformations);
	bpmtext->setFont(m_bpmFont);
	//: %1 = tempo in beats per minute, %2/%3 = time signature
	bpmtext->setText(tr("%1 %2/%3").arg(ti.beatsPerSecond * 60).arg(ti.beatsPerBar).arg(ti.beatsPerWholeNote));
	QColor color("black"); color.setAlpha(64);
	bpmtext->setBrush(color);

	double pixelsPerBar = ti.beatsPerBar / ti.beatsPerSecond;
	int lastbar = (int)floor(rect().width() / pixelsPerBar);
	qreal h = rect().height();

	for (int b=0; b<=lastbar; b++)
	{
		qreal x = b * pixelsPerBar;

		QGraphicsRectItem* child = new QGraphicsRectItem(0, 0, pixelsPerBar, h, this);
		child->setPos(x, 0);
		child->setZValue(zRulerBar);
		child->setPen(Qt::NoPen);

		enum {none, small, large} grid = none;

		if (b % ti.barsPerSmallGrid == 0)
		{
			if (b % (ti.barsPerSmallGrid * ti.smallGridsPerLargeGrid) == 0)
			{
				grid = large;
				child->setBrush(m_colLargeGrid);
			}
			else
			{
				grid = small;
				child->setBrush(m_colSmallGrid);
			}
		}
		else if (b % 2 == 0)
			child->setBrush(m_colEven);
		else
			child->setBrush(m_colOdd);

		m_beatRects.push_back(child);

		if (grid != none)
		{
			GraphicsSimpleTextItemWithBG* label = new GraphicsSimpleTextItemWithBG(this);
			label->setZValue(zRulerLabel);
			label->setFlag(QGraphicsItem::ItemIgnoresTransformations);
			label->setText(QString(" %1 ").arg(b));
			label->setPos(x, h - label->boundingRect().height());
			if (grid == large)
				label->setBrush(m_colLargeGridLabel);
			else
				label->setBrush(m_colSmallGridLabel);
			label->setBgBrush(child->brush());

			m_gridLabels.insert(b, label);
		}
	}
}

double RulerSectionItem::getNearestSnapPoint(double x)
{
	double secondsPerSnap = m_mtc->getTimeInfo().beatsPerBar / m_mtc->getTimeInfo().beatsPerSecond;
	return floor(x / secondsPerSnap + 0.5) * secondsPerSnap;
}

//////////////////////////////////////////////////////////////////////////////

PlayLineItem::PlayLineItem(SequenceEditor::Editor* editor, qreal height)
: m_editor(editor), QGraphicsLineItem(0,0,0,height)
{
	QPen pen(Theme::get().getColor("SequenceEditor/PlayLine"), 3);
	pen.setCosmetic(true);
	setPen(pen);
	setOpacity(0.6);

	connect(editor, SIGNAL(signalChangePlayPos(double)),
		this, SLOT(setPlayPos(double)) );
}

void PlayLineItem::setHeight(qreal height)
{
	setLine(0,0,0,height);
}

void PlayLineItem::setPlayPos(double seconds)
{
	setPos(seconds, 0);
}

///////////////////////////////////////////////////////////////////////////////

LoopMarkerItem::LoopMarkerItem(Editor* editor, qreal height, Type type)
: m_editor(editor)
{
	setPen(Qt::NoPen);
	setBrush(Qt::black);

	setFlag(ItemIgnoresTransformations);
	if (type == loopEnd) scale(-1, 1);

	QPainterPath path;
	path.addRect(0, 0, 3, height);
	path.addRect(5, 0, 1, height);
	path.addEllipse(8, height * 1.0/3.0, 3, 3);
	path.addEllipse(8, height * 2.0/3.0, 3, 3);

	setPath(path);
}
