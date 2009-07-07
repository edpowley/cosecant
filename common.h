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

inline double remap(double x, double oldmin, double oldmax, double newmin, double newmax)
{
	return (x-oldmin) / (oldmax-oldmin) * (newmax-newmin) + newmin;
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
