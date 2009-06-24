#pragma once

/** A 2-dimensional vector. Used to represent both positions and sizes. */
template<typename Scalar>
class Vector2
{
public:
	/** The x (horizontal) component. */
	Scalar x;

	/** The y (vertical) component. */
	Scalar y;

	/** Default constructor sets both components to zero. */
	Vector2() { x = y = 0; }

	/** Constructor. */
	Vector2(Scalar x_, Scalar y_) { x = x_; y = y_; }
	
	/** Assignment operator. */
	Vector2& operator=(const Vector2& rhs)
	{ x = rhs.x; y = rhs.y; return *this; }

	/** In-place vector addition. */
	Vector2& operator+=(const Vector2& rhs)
	{ x += rhs.x; y += rhs.y; return *this; }

	/** In-place vector subtraction. */
	Vector2& operator-=(const Vector2& rhs)
	{ x -= rhs.x; y -= rhs.y; return *this; }

	/** Vector addition. */
	Vector2 operator+(const Vector2& rhs) const
	{ return Vector2(x + rhs.x, y + rhs.y); }

	/** Vector subtraction. */
	Vector2 operator-(const Vector2& rhs) const
	{ return Vector2(x - rhs.x, y - rhs.y); }

	/** In-place vector multiplication by a scalar. */
	Vector2& operator*=(Scalar rhs)
	{ x *= rhs; y *= rhs; return *this; }

	/** In-place vector division by a scalar. */
	Vector2& operator/=(Scalar rhs)
	{ x /= rhs; y /= rhs; return *this; }

	/** Vector multiplication by a scalar. */
	Vector2 operator*(Scalar rhs) const
	{ return Vector2(x * rhs, y * rhs); }

	/** Vector division by a scalar. */
	Vector2 operator/(Scalar rhs) const
	{ return Vector2(x / rhs, y / rhs); }

	/** Unary negation */
	Vector2 operator-() const
	{ return Vector2(-x, -y); }

	/** Equality operator. */
	bool operator==(const Vector2& rhs) const
	{ return x == rhs.x && y == rhs.y; }

	/** Inequality operator. */
	bool operator!=(const Vector2& rhs) const
	{ return x != rhs.x || y != rhs.y; }

	/** In-place round down to the nearest integer coordinates. */
	void floor() { x = ::floor(x); y = ::floor(y); }

	/** Element-wise multiplication. */
	Vector2 multElementWise(const Vector2& rhs)
	{ return Vector2(x * rhs.x, y * rhs.y); }

	/** Square of length. */
	Scalar lengthSquared()
	{ return x*x + y*y; }

	/** Length. */
	Scalar length()
	{ return sqrt(lengthSquared()); }

	/** Unit vector. */
	Vector2 unit()
	{ return *this / length(); }

	/** Dot product */
	Scalar dot(const Vector2& other)
	{
		return x*other.x + y*other.y;
	}
};

//////////////////////////////////////////////////////////////////////////////////

/** A 2-dimensional axis-aligned rectangle. */
template<typename Scalar>
class Rect2
{
	friend Rect2<Scalar> RectLTRB(Scalar, Scalar, Scalar, Scalar);
public:
	/** Default constructor sets everything to zero. */
	Rect2() {}

	/** Constructor. */
	Rect2(const Vector2& topleft, const Vector2& size)
		: m_topleft(topleft), m_size(size) { normalise(); }

	/** Constructor. */
	Rect2(Scalar x, Scalar y, Scalar w, Scalar h)
		: m_topleft(x,y), m_size(w,h) { normalise(); }

	/** Assignment operator. */
	Rect2& operator=(const Rect2& rhs)
	{ m_topleft = rhs.m_topleft; m_size = rhs.m_size; return *this; }

	/** In-place translation by a vector. */
	Rect2& operator+=(const Vector2& rhs)
	{ m_topleft += rhs; return *this; }

	/** In-place translation by a vector. */
	Rect2& operator-=(const Vector2& rhs)
	{ m_topleft -= rhs; return *this; }

	/** Translation by a vector. */
	Rect2 operator+(const Vector2& rhs) const
	{ return Rect2(m_topleft + rhs, m_size); }

	/** Translation by a vector. */
	Rect2 operator-(const Vector2& rhs) const
	{ return Rect2(m_topleft - rhs, m_size); }

	/** Ensure that the top left vector really is the top left, and that the width and
		height are non-negative. */
	void normalise();

	/** Return the x coordinate of the left edge. */
	Scalar left()	const { return m_topleft.x; }

	/** Return the y coordinate of the top edge. */
	Scalar top()	const { return m_topleft.y; }

	/** Return the x coordinate of the right edge. */
	Scalar right()	const { return m_topleft.x + m_size.x; }

	/** Return the y coordinate of the bottom edge. */
	Scalar bottom()	const { return m_topleft.y + m_size.y; }

	/** Return the position of the top left corner. */
	Vector2 topleft()		const { return Vector2(left(), top()); }

	/** Return the position of the top right corner. */
	Vector2 topright()		const { return Vector2(right(), top()); }

	/** Return the position of the bottom left corner. */
	Vector2 bottomleft()	const { return Vector2(left(), bottom()); }

	/** Return the position of the bottom right corner. */
	Vector2 bottomright()	const { return Vector2(right(), bottom()); }

	Vector2 size() const { return m_size; }
	Scalar width() const { return m_size.x; }
	Scalar height() const { return m_size.y; }

	/** Set the left, top, right and bottom coordinates. */
	void setLTRB(Scalar left, Scalar top, Scalar right, Scalar bottom)
	{ m_topleft = Vector2(left,top); m_size = Vector2(right-left, bottom-top); }

	/** Set the x coordinate of the left edge. */
	void setLeft(Scalar s);

	/** Set the y coordinate of the top edge. */
	void setTop(Scalar s);

	/** Set the x coordinate of the right edge. */
	void setRight(Scalar s);

	/** Set the y coordinate of the bottom edge. */
	void setBottom(Scalar s);

	/** Expand (in-place) this rectangle so that it contains another rectangle. */
	void unite(const Rect2& other);

	/** Expand (in-place) this rectangle so that it contains another point. */
	void unite(const Vector2& point);

protected:
	Vector2 m_topleft, m_size;
};

/** Construct a rectangle from left, top, right and bottom coordinates. */
template<typename Scalar>
inline Rect2<Scalar> Rect2LTRB(Scalar left, Scalar top, Scalar right, Scalar bottom)
{
	Rect2 r; r.setLTRB(left, top, right, bottom); r.normalise(); return r;
}

//////////////////////////////////////////////////////////////////////////////////

typedef Vector2<double> Vector2f;
typedef Rect2<double> Rect2f;

/// Point unpack macro
#define PUP(xxx)  xxx.x, xxx.y

//////////////////////////////////////////////////////////////////////////////////

/** Returns a vector whose y component is the perpendicular distance from point p to line p1-p2,
	and whose x component is the point where the perpendicular meets the line with 0 being p1 and 1 being p2.
*/
Vector2f calcPointLinePositionDistance(Vector2f p, Vector2f p1, Vector2f p2);
