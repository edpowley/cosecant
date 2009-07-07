#include "stdafx.h"
#include "common.h"

static QHash<QString, QChar> g_htmlEntityMap;
QHash<QString, QChar> getHtmlEntityMap(); // in html_entities/htmlentitymap.cpp

void initHtmlEntityMap()
{
	g_htmlEntityMap = getHtmlEntityMap();
}

QString convertHtmlEntitiesToUnicode(const QString& ss)
{
	QString s = ss;
	QRegExp re("&([^;]*);");

	for (int index = s.indexOf(re); index != -1; index = s.indexOf(re, index+1))
	{
		qDebug() << s << index << re.cap(1);

		QChar ch(0);

		if (re.cap(1).startsWith('#'))
		{
			if (re.cap(1).startsWith("#x", Qt::CaseInsensitive)) // hex
			{
				bool ok = false;
				ch = re.cap(1).mid(2).toUInt(&ok, 16);
				if (!ok) ch = 0;
			}
			else // decimal
			{
				bool ok = false;
				ch = re.cap(1).mid(1).toUInt(&ok, 10);
				if (!ok) ch = 0;
			}
		}
		else
		{
			ch = g_htmlEntityMap.value(re.cap(1), QChar(0));
		}

		if (!ch.isNull())
		{
			s.replace(index, re.matchedLength(), ch);
		}
	}

	return s;
}
