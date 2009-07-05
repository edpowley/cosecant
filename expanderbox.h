#pragma once

class ExpanderBox : public QGroupBox
{
	Q_OBJECT

public:
	ExpanderBox(const QString& title, QWidget* parent = NULL);

	QWidget* getContent() { return m_content; }

protected:
	QWidget* m_content;
};
