#include "stdafx.h"
#include <QStyleOptionGraphicsItem>
#include "common.h"
#include "mygraphicsview.h"

PrefsVar_Bool MyGraphicsView::s_prefAntiAlias("graphics/view/aa", true);

MyGraphicsView::MyGraphicsView(QWidget* parent)
: QGraphicsView(parent), m_handleCursorKeys(true)
{
	ctorCommon();
}

MyGraphicsView::MyGraphicsView(QGraphicsScene* scene, QWidget* parent)
: QGraphicsView(scene, parent)
{
	ctorCommon();
}

void MyGraphicsView::ctorCommon()
{
	setup();

	connect(&s_prefAntiAlias, SIGNAL(signalChange()),
		this, SLOT(onPrefsChange()) );
}

void MyGraphicsView::onPrefsChange()
{
	setup();
}

void MyGraphicsView::setup()
{
	QPainter::RenderHints h = QPainter::TextAntialiasing;
	if (s_prefAntiAlias())
		h |= QPainter::Antialiasing;
	setRenderHints(h);
}

void MyGraphicsView::resizeEvent(QResizeEvent* ev)
{
	QGraphicsView::resizeEvent(ev);
	signalResize(ev->oldSize(), ev->size());
}

void MyGraphicsView::keyPressEvent(QKeyEvent* ev)
{
	if (m_handleCursorKeys)
		QGraphicsView::keyPressEvent(ev);
	else
		QWidget::keyPressEvent(ev);
}

////////////////////////////////////////////////////////////////////////////////

void GraphicsSimpleTextItemWithBG::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(Qt::NoPen);
	painter->setBrush(m_bgBrush);
	painter->drawRect(boundingRect());

	QGraphicsSimpleTextItem::paint(painter, option, widget);
}

///////////////////////////////////////////////////////////////////////////////

GraphicsLayoutTextItem::GraphicsLayoutTextItem(const QRectF& rect,
											   int flags,
											   const QString& text,
											   QGraphicsItem* parent)
:	QGraphicsItem(parent),
	m_rect(rect), m_boundingRect(rect),
	m_flags(flags), m_text(text)
{
}

QRectF GraphicsLayoutTextItem::boundingRect() const
{
	return m_boundingRect;
}

void GraphicsLayoutTextItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->save();

	QMatrix mat = option->matrix;
	QMatrix imat = mat.inverted();
	painter->setTransform(QTransform(imat), true);

	painter->setFont(m_font);
	painter->setBrush(Qt::NoBrush);
	painter->setPen(m_pen);
	painter->drawText(mat.mapRect(m_rect), m_flags, m_text, &m_boundingRect);
	m_boundingRect = imat.mapRect(m_boundingRect);

	painter->restore();
}
