#pragma once

#include "svnrevision.h"

static const int versionMajor = 0;
static const int versionMinor = 0;
static const char* versionGreek = "alpha";

inline QString getVersionString()
{
	QString s = QString("%1.%2.%3").arg(versionMajor).arg(versionMinor).arg(versionSvn);
	
	if (versionSvnModified)
		s += "m";

	if (versionGreek[0] != 0)
		s += QString(" ") + versionGreek;

#	ifdef _DEBUG
		s += " debug build";
#	endif

	return s;
}
