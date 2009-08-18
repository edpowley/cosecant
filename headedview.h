#pragma once

#include "mygraphicsview.h"

class HeadedView : public QSplitter
{
	Q_OBJECT

public:
	HeadedView(QWidget* parent = NULL);
	
	static const int c_rulerHeight = 50;

protected:
	MyGraphicsView *m_headView, *m_bodyView, *m_rulerView;

	void showEvent(QShowEvent* ev);
	bool m_firstShow;

private:
	QVBoxLayout *m_lhead, *m_lbody;
	QHBoxLayout *m_lruler;
};
