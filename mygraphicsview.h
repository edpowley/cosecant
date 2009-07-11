#pragma once

#include "prefs.h"

class MyGraphicsView : public QGraphicsView
{
	Q_OBJECT

public:
	static PrefsVar_Bool s_prefAntiAlias;

	MyGraphicsView(QWidget* parent = 0);
	MyGraphicsView(QGraphicsScene* scene, QWidget* parent = 0);

protected slots:
	void onPrefsChange();

protected:
	void ctorCommon();
	void setup();
};
