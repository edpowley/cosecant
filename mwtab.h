#pragma once

class MWTab
{
public:
	virtual QWidget* getMWTabWidget() = 0;
	virtual QString getTitle() = 0;

	virtual QList<QAction*> getToolBarActions() { return QList<QAction*>(); }
	virtual QWidget* getPalette() { return NULL; }

	void showTab() {}
};
