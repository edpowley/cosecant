#include "stdafx.h"
#include "common.h"
#include "mygraphicsview.h"

PrefsVar_Bool MyGraphicsView::s_prefAntiAlias("graphics/view/aa", true);

MyGraphicsView::MyGraphicsView(QWidget* parent)
: QGraphicsView(parent)
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

////////////////////////////////////////////////////////////////////////////////

void GraphicsSimpleTextItemWithBG::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	painter->setPen(Qt::NoPen);
	painter->setBrush(m_bgBrush);
	painter->drawRect(boundingRect());

	QGraphicsSimpleTextItem::paint(painter, option, widget);
}
