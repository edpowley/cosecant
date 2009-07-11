#include "stdafx.h"
#include "common.h"
#include "mygraphicsview.h"

PrefsVar_Bool MyGraphicsView::s_prefOpenGL("graphics/view/gl", false);
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
	setupGlAndAa();

	connect(&s_prefOpenGL, SIGNAL(signalChange()),
		this, SLOT(onPrefsChange()) );
	connect(&s_prefAntiAlias, SIGNAL(signalChange()),
		this, SLOT(onPrefsChange()) );
}

void MyGraphicsView::onPrefsChange()
{
	setupGlAndAa();
}

void MyGraphicsView::setupGlAndAa()
{
	if (s_prefOpenGL())
		setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
	else
		setViewport(new QWidget);

	QPainter::RenderHints h = QPainter::TextAntialiasing;
	if (s_prefAntiAlias())
		h |= QPainter::Antialiasing;
	setRenderHints(h);
}
