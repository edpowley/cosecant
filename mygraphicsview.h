#pragma once

#include "prefs.h"

class MyGraphicsView : public QGraphicsView
{
	Q_OBJECT

public:
	static PrefsVar_Bool s_prefAntiAlias;

	MyGraphicsView(QWidget* parent = 0);
	MyGraphicsView(QGraphicsScene* scene, QWidget* parent = 0);

	void setHandleCursorKeys(bool b) { m_handleCursorKeys = b; }

signals:
	void signalResize(const QSize& oldsize, const QSize& newsize);

protected slots:
	void onPrefsChange();

protected:
	void ctorCommon();
	void setup();

	void resizeEvent(QResizeEvent* ev);

	bool m_handleCursorKeys;
	void keyPressEvent(QKeyEvent* ev);
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

class GraphicsLayoutTextItem : public QGraphicsItem
{
public:
	GraphicsLayoutTextItem(const QRectF& rect, int flags, const QString& text, QGraphicsItem* parent = 0);

	void setFont(const QFont& font) { m_font = font; update(); }
	void setPen (const QPen& pen)   { m_pen  = pen;  update(); }
	void setText(const QString& text) { m_boundingRect = m_rect; m_text = text; update(); }

	virtual QRectF boundingRect() const;
	virtual void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget = 0);

protected:
	QRectF m_rect, m_boundingRect;
	int m_flags;
	QString m_text;
	QFont m_font;
	QPen m_pen;
};
