#pragma once

// TODO move this somewhere
namespace frame
{
    enum class Side
    {
        Left,
        Right
    };
}

class Point
{
public:
    Point();
    Point(Point&& o);

    Point& operator=(const Point& o);
    Point& operator=(Point&& o);

    Point(const Point& o);
    //Point& operator+=(const Point& o);
    //Point& operator-=(const Point& o);
    //Point operator+(const Point& o) const;
    //Point operator-(const Point& o) const;

    template<class T> Point(T x, T y);
    template<class T> Point& operator+=(T o);
    template<class T> Point& operator-=(T o);
    template<class T> Point& operator*=(T o);
    template<class T> Point& operator/=(T o);
    template<class T> Point operator+(T o) const;
    template<class T> Point operator-(T o) const;
    template<class T> Point operator*(T o) const;
    template<class T> Point operator/(T o) const;

    void Normalize();
    Point Normalized() const;

    void Rotate(float radians);
    Point Rotated(float radians) const;

    float Angle() const;
    float Length() const;

    Point Perpendicular(frame::Side side) const;

    Point Interpolate(const Point& o, float n);

    float x() const;
    float y() const;

private:
    float m_x;
    float m_y;
};

// implementation of template methods

template<class T>
Point::Point(T x, T y)
    : m_x((float)x), m_y((float)y)
{
}

template<class T>
Point& Point::operator+=(T o)
{
    m_x += (float)o;
    m_y += (float)o;
    return *this;
}

template<class T>
Point& Point::operator-=(T o)
{
    m_x -= (float)o;
    m_y -= (float)o;
    return *this;
}

template<class T>
Point& Point::operator*=(T o)
{
    m_x *= (float)o;
    m_y *= (float)o;
    return *this;
}

template<class T>
Point& Point::operator/=(T o)
{
    m_x /= (float)o;
    m_y /= (float)o;
    return *this;
}

template<class T>
Point Point::operator+(T o) const
{
    return { m_x + o, m_y + o };
}

template<class T>
Point Point::operator-(T o) const
{
    return { m_x - o, m_y - o };
}

template<class T>
Point Point::operator*(T o) const
{
    return { m_x * o, m_y * o };
}

template<class T>
Point Point::operator/(T o) const
{
    return { m_x / o, m_y / o };
}

// specializations for Point type

template<> Point& Point::operator+=<const Point&>(const Point& o);
template<> Point& Point::operator-=<const Point&>(const Point& o);
template<> Point& Point::operator*=<const Point&>(const Point& o);
template<> Point& Point::operator/=<const Point&>(const Point& o);
template<> Point Point::operator+<Point>(Point o) const;
template<> Point Point::operator-<Point>(Point o) const;
template<> Point Point::operator*<Point>(Point o) const;
template<> Point Point::operator/<Point>(Point o) const;
