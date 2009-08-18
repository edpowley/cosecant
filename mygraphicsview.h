#pragma once

#include "prefs.h"

class MyGraphicsView : public QGraphicsView
{
	Q_OBJECT

public:
	static PrefsVar_Bool s_prefAntiAlias;

	MyGraphicsView(QWidget* parent = 0);
	MyGraphicsView(QGraphicsScene* scene, QWidget* parent = 0);

signals:
	void signalResize(const QSize& oldsize, const QSize& newsize);

protected slots:
	void onPrefsChange();

protected:
	void ctorCommon();
	void setup();

	void resizeEvent(QResizeEvent* ev);
};

class GraphicsSimpleTextItemWithBG : public QGraphicsSimpleTextItem
{
public:
	GraphicsSimpleTextItemWithBG(QGraphicsItem* parent = 0)
		: QGraphicsSimpleTextItem(parent) {}
	GraphicsSimpleTextItemWithBG(const QString& text, QGraphicsItem* parent = 0)
		: QGraphicsSimpleTextItem(text, parent) {}

	void setBgBrush(const QBrush& brush) { m_bgBrush = brush; }
	QBrush bgBrush() { return m_bgBrush; }

	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

protected:
	QBrush m_bgBrush;
};
