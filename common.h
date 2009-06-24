#pragma once

#include "error.h"
#include "object.h"
#include "utility.h"

template<typename T> T min(T a, T b) { return (a < b) ? a : b; }
template<typename T> T max(T a, T b) { return (a > b) ? a : b; }

template<typename T> T clamp(T x, T a, T b)
{
	return (x < a) ? a : ((x > b) ? b : x);
}

inline int truemod(int a, int b)
{
	int m = a % b;
	if (m < 0)
		return m + b;
	else
		return m;
}

inline std::ostream& operator<<(std::ostream& stream, const QString& str)
{
	stream << str.toAscii().constData();
	return stream;
}

inline QPointF multElementWise(const QPointF& a, const QPointF& b)
{
	return QPointF(a.x() * b.x(), a.y() * b.y());
}

/////////////////////////////////////////////////////////////////////

// TODO: remove stuff below

#include "wustring.h"

inline Glib::ustring q2ustring(const QString& str)
{
	return str.toUtf8().constData();
}
