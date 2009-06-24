#include "stdafx.h"
#include "common.h"
#include "geometry.h"

Vector2f calcPointLinePositionDistance(Vector2f p, Vector2f p1, Vector2f p2)
{
	if (p1 == p2) // the "line" is actually a point
		return Vector2f(0.0, (p1-p).length());

	// http://mathworld.wolfram.com/Point-LineDistance2-Dimensional.html

	Vector2f v(p2.y - p1.y, p1.x - p2.x);
	Vector2f vhat = v.unit();
	Vector2f r = p - p1;
	double d = abs(vhat.dot(r));

	Vector2f u = p2-p1;
	double ulen = u.length();
	Vector2f uhat = u.unit();
	double pos = uhat.dot(r) / ulen;

	return Vector2f(pos, d);
}
