#pragma once

#include "mwtab.h"
#include "sequence.h"

class PatternEditor : public QObject, public MWTab
{
public:
	PatternEditor(const Ptr<Sequence::Pattern>& pattern);
	virtual ~PatternEditor();

	virtual QWidget* getMWTabWidget() { return m_widget; }
	virtual QString getTitle();

	virtual QToolBar* getToolBar() { return NULL; }

protected:
	Ptr<Sequence::Pattern> m_pattern;
	QWidget* m_widget;
};
