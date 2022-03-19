#include "point.h"
#include <cmath>

Point::Point()
    : m_x(0.0f), m_y(0.0f)
{

}

Point::Point(const Point& o)
    : m_x(o.m_x), m_y(o.m_y)
{

}

Point::Point(Point&& o)
    : m_x(o.m_x), m_y(o.m_y)
{

}

Point& Point::operator=(const Point& o)
{
    m_x = o.m_x;
    m_y = o.m_y;
    return *this;
}

Point& Point::operator=(Point&& o)
{
    m_x = o.m_x;
    m_y = o.m_y;
    return *this;
}

void Point::Normalize()
{
    float len = Length();
    m_x /= len;
    m_y /= len;
}

Point Point::Normalized() const
{
    Point p(*this);
    p.Normalize();
    return p;
}

void Point::Rotate(float radians)
{
    float s = std::sinf(radians);
    float c = std::cosf(radians);

    float x = m_x * c - m_y * s;
    float y = m_y * c + m_x * s;

    m_x = x;
    m_y = y;
}

Point Point::Rotated(float radians) const
{
    Point p(*this);
    p.Rotate(radians);
    return p;
}

float Point::Angle() const
{
    float result = std::atan2f(m_y, m_x);

    return result < 0.0f ? result + 6.2831853f : result;
}

float Point::Length() const
{
    return std::sqrtf(m_x*m_x + m_y*m_y);
}

float Point::x() const
{
    return m_x;
}

float Point::y() const
{
    return m_y;
}

Point Point::Interpolate(const Point& o, float n)
{
    return { m_x + (o.m_x - m_x) * n, m_y + (o.m_y - m_y) * n };
}

Point Point::Perpendicular(frame::Side side) const
{
    if (side == frame::Side::Left)
        return { -m_y, m_x };
    return { m_y, -m_x };
}

template<>
Point& Point::operator+=<const Point&>(const Point& o)
{
    m_x += o.m_x;
    m_y += o.m_y;
    return *this;
}

template<>
Point& Point::operator-=<const Point&>(const Point& o)
{
    m_x -= o.m_x;
    m_y -= o.m_y;
    return *this;
}

template<>
Point& Point::operator*=<const Point&>(const Point& o)
{
    m_x *= o.m_x;
    m_y *= o.m_y;
    return *this;
}

template<>
Point& Point::operator/=<const Point&>(const Point& o)
{
    m_x /= o.m_x;
    m_y /= o.m_y;
    return *this;
}

template<>
Point Point::operator+<Point>(Point o) const
{
    return { m_x + o.m_x, m_y + o.m_y };
}

template<>
Point Point::operator-<Point>(Point o) const
{
    return { m_x - o.m_x, m_y - o.m_y };
}

template<>
Point Point::operator*<Point>(Point o) const
{
    return { m_x * o.m_x, m_y * o.m_y };
}

template<>
Point Point::operator/<Point>(Point o) const
{
    return { m_x / o.m_x, m_y / o.m_y };
}
