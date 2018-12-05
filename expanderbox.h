#pragma once

#include <QGroupBox>

class ExpanderBox : public QGroupBox
{
	Q_OBJECT

public:
	ExpanderBox(const QString& title, QWidget* parent = NULL);

	QWidget* getContent() { return m_content; }

protected:
	QWidget* m_content;
	QTimeLine* m_showTimeLine;
	QTimeLine* m_hideTimeLine;

protected slots:
	void onToggled(bool toggle);
	void setContentHeight(int h);
	void unsetContentHeight();
};
