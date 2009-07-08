#pragma once

class CursorRaii
{
public:
	CursorRaii(Qt::CursorShape shape)
	{
		QApplication::setOverrideCursor(QCursor(shape));
	}

	~CursorRaii()
	{
		QApplication::restoreOverrideCursor();
	}
};
