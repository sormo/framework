#pragma once

// TODO move this somewhere
namespace frame
{
    enum class side
    {
        left,
        right
    };
}

template<typename M>
class point_type
{
public:
    point_type();
    point_type(point_type&& o);

    point_type& operator=(const point_type& o);
    point_type& operator=(point_type&& o);

    point_type(const point_type& o);
    //Point& operator+=(const Point& o);
    //Point& operator-=(const Point& o);
    //Point operator+(const Point& o) const;
    //Point operator-(const Point& o) const;

    point_type point_type::operator-() const;

    template<class T> point_type(T x, T y);
    template<class T> point_type& operator+=(T o);
    template<class T> point_type& operator-=(T o);
    template<class T> point_type& operator*=(T o);
    template<class T> point_type& operator/=(T o);
    point_type& operator+=(const point_type& o);
    point_type& operator-=(const point_type& o);
    point_type& operator*=(const point_type& o);
    point_type& operator/=(const point_type& o);
    template<class T> point_type operator+(T o) const;
    template<class T> point_type operator-(T o) const;
    template<class T> point_type operator*(T o) const;
    template<class T> point_type operator/(T o) const;
    point_type operator+(const point_type& o) const;
    point_type operator-(const point_type& o) const;
    point_type operator*(const point_type& o) const;
    point_type operator/(const point_type& o) const;

    void normalize();
    point_type normalized() const;

    void rotate(M radians);
    point_type rotated(M radians) const;

    M angle() const;
    M length() const;
    M length_sqr() const;

    M cross(const point_type& o) const;
    M dot(const point_type& o) const;

    point_type abs() const; // return absolute value
    point_type sign() const;

    point_type perpendicular(frame::side side) const;

    point_type interpolate(const point_type& o, M n);

    M x;
    M y;
};

// implementation of template methods

template<class M>
point_type<M> point_type<M>::operator-() const
{
    return { -x, -y };
}

template<class M>
template<class T>
point_type<M>::point_type(T x, T y)
    : x((M)x), y((M)y)
{
}

template<class M>
template<class T>
point_type<M>& point_type<M>::operator+=(T o)
{
    x += (M)o;
    y += (M)o;
    return *this;
}

template<class M>
template<class T>
point_type<M>& point_type<M>::operator-=(T o)
{
    x -= (M)o;
    y -= (M)o;
    return *this;
}

template<class M>
template<class T>
point_type<M>& point_type<M>::operator*=(T o)
{
    x *= (M)o;
    y *= (M)o;
    return *this;
}

template<class M>
template<class T>
point_type<M>& point_type<M>::operator/=(T o)
{
    x /= (M)o;
    y /= (M)o;
    return *this;
}

//

template<class M>
point_type<M>& point_type<M>::operator+=(const point_type<M>& o)
{
    x += o.x;
    y += o.y;
    return *this;
}

template<class M>
point_type<M>& point_type<M>::operator-=(const point_type<M>& o)
{
    x -= o.x;
    y -= o.y;
    return *this;
}

template<class M>
point_type<M>& point_type<M>::operator*=(const point_type<M>& o)
{
    x *= o.x;
    y *= o.y;
    return *this;
}

template<class M>
point_type<M>& point_type<M>::operator/=(const point_type<M>& o)
{
    x /= o.x;
    y /= o.y;
    return *this;
}

//

template<class M>
template<class T>
point_type<M> point_type<M>::operator+(T o) const
{
    return { x + o, y + o };
}

template<class M>
template<class T>
point_type<M> point_type<M>::operator-(T o) const
{
    return { x - o, y - o };
}

template<class M>
template<class T>
point_type<M> point_type<M>::operator*(T o) const
{
    return { x * o, y * o };
}

template<class M>
template<class T>
point_type<M> point_type<M>::operator/(T o) const
{
    return { x / o, y / o };
}

//

template<class M>
point_type<M> point_type<M>::operator+(const point_type& o) const
{
    return { x + o.x, y + o.y };
}

template<class M>
point_type<M> point_type<M>::operator-(const point_type& o) const
{
    return { x - o.x, y - o.y };
}

template<class M>
point_type<M> point_type<M>::operator*(const point_type& o) const
{
    return { x * o.x, y * o.y };
}

template<class M>
point_type<M> point_type<M>::operator/(const point_type& o) const
{
    return { x / o.x, y / o.y };
}

//

template<class M>
point_type<M>::point_type()
    : x(0.0f), y(0.0f)
{
}

template<class M>
point_type<M>::point_type(const point_type& o)
    : x(o.x), y(o.y)
{
}

template<class M>
point_type<M>::point_type(point_type&& o)
    : x(o.x), y(o.y)
{
}


template<class M>
point_type<M>& point_type<M>::operator=(const point_type<M>& o)
{
    x = o.x;
    y = o.y;
    return *this;
}


template<class M>
point_type<M>& point_type<M>::operator=(point_type<M>&& o)
{
    x = o.x;
    y = o.y;
    return *this;
}

template<class M>
void point_type<M>::normalize()
{
    M len = length();
    x /= len;
    y /= len;
}

template<class M>
point_type<M> point_type<M>::normalized() const
{
    point_type p(*this);
    p.normalize();
    return p;
}

template<class M>
void point_type<M>::rotate(M radians)
{
    M s = std::sin(radians);
    M c = std::cos(radians);

    M x = x * c - y * s;
    M y = y * c + x * s;

    x = x;
    y = y;
}

template<class M>
point_type<M> point_type<M>::rotated(M radians) const
{
    point_type p(*this);
    p.rotate(radians);
    return p;
}

template<class M>
M point_type<M>::angle() const
{
    M result = std::atan2(y, x);

    return result < 0.0f ? result + 6.2831853f : result;
}

template<class M>
M point_type<M>::length() const
{
    return std::sqrt(x * x + y * y);
}

template<class M>
point_type<M> point_type<M>::interpolate(const point_type<M>& o, M n)
{
    return { x + (o.x - x) * n, y + (o.y - y) * n };
}

template<class M>
point_type<M> point_type<M>::perpendicular(frame::side side) const
{
    if (side == frame::side::left)
        return { -y, x };
    return { y, -x };
}

template<class M>
point_type<M> point_type<M>::abs() const
{
    return { std::abs(x), std::abs(y) };
}

template<class M>
point_type<M> point_type<M>::sign() const
{
    return { x < M(0.0) ? M(-1.0) : M(1.0), y < M(0.0) ? M(-1.0) : M(1.0) };
}

template<class M>
M point_type<M>::length_sqr() const
{
    return x * x + y * y;
}

template<class M>
M point_type<M>::cross(const point_type& o) const
{
    return a.x * b.y - b.x * a.y;
}

template<class M>
M point_type<M>::dot(const point_type& o) const
{
    return x * o.x + y * o.y;
}
