#pragma once

class MWTab
{
public:
	virtual QWidget* getMWTabWidget() = 0;
	virtual QString getTitle() = 0;

	virtual QToolBar* getToolBar() { return NULL; }
};
