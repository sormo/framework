#pragma once
#include <cmath>
#include <algorithm>

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

    point_type operator-() const;

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
    M s = sin(radians);
    M c = cos(radians);

    M x_new = x * c - y * s;
    M y_new = y * c + x * s;

    x = x_new;
    y = y_new;
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
    return sqrt(x * x + y * y);
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
    return x * o.y - o.x * y;
}

template<class M>
M point_type<M>::dot(const point_type& o) const
{
    return x * o.x + y * o.y;
}

// --- 
// TODO duplication

template<typename M>
class point_type_3
{
public:
    point_type_3();
    point_type_3(const point_type_3& o);
    point_type_3(point_type_3&& o);

    point_type_3& operator=(const point_type_3& o);
    point_type_3& operator=(point_type_3&& o);

    template<class T> point_type_3(const point_type<T>& o);
    //Point& operator+=(const Point& o);
    //Point& operator-=(const Point& o);
    //Point operator+(const Point& o) const;
    //Point operator-(const Point& o) const;

    template<class T> point_type<T> xy() const;
    template<class T> point_type<T> xz() const;
    template<class T> point_type<T> yz() const;

    point_type_3 operator-() const;

    template<class T> point_type_3(T x, T y, T z);
    template<class T> point_type_3& operator+=(T o);
    template<class T> point_type_3& operator-=(T o);
    template<class T> point_type_3& operator*=(T o);
    template<class T> point_type_3& operator/=(T o);
    point_type_3& operator+=(const point_type_3& o);
    point_type_3& operator-=(const point_type_3& o);
    point_type_3& operator*=(const point_type_3& o);
    point_type_3& operator/=(const point_type_3& o);
    template<class T> point_type_3 operator+(T o) const;
    template<class T> point_type_3 operator-(T o) const;
    template<class T> point_type_3 operator*(T o) const;
    template<class T> point_type_3 operator/(T o) const;
    point_type_3 operator+(const point_type_3& o) const;
    point_type_3 operator-(const point_type_3& o) const;
    point_type_3 operator*(const point_type_3& o) const;
    point_type_3 operator/(const point_type_3& o) const;

    void normalize();
    point_type_3 normalized() const;

    //void rotate(M radians);
    // normal is vector to rotate around (normal vector of rotation plane
    point_type_3 rotated(M radians, const point_type_3& normal) const;

    M angle(const point_type_3& o) const;

    M length() const;
    M length_sqr() const;

    point_type_3 cross(const point_type_3& o) const;
    M dot(const point_type_3& o) const;

    point_type_3 abs() const; // return absolute value
    point_type_3 sign() const;

    //point_type_3 perpendicular(frame::side side) const;

    //point_type_3 interpolate(const point_type_3& o, M n);

    M x;
    M y;
    M z;
};

// implementation of template methods
template<class M>
template<class T>
point_type<T> point_type_3<M>::xy() const
{
    return { x, y };
}

template<class M>
template<class T>
point_type<T> point_type_3<M>::xz() const
{
    return { x, z };
}

template<class M>
template<class T>
point_type<T> point_type_3<M>::yz() const
{
    return { y, z };
}

template<class M>
point_type_3<M> point_type_3<M>::operator-() const
{
    return { -x, -y, -z };
}

template<class M>
template<class T>
point_type_3<M>::point_type_3(T x, T y, T z)
    : x((M)x), y((M)y), z((M)z)
{
}

template<class M>
template<class T>
point_type_3<M>& point_type_3<M>::operator+=(T o)
{
    x += (M)o;
    y += (M)o;
    z += (M)o;
    return *this;
}

template<class M>
template<class T>
point_type_3<M>& point_type_3<M>::operator-=(T o)
{
    x -= (M)o;
    y -= (M)o;
    z -= (M)o;
    return *this;
}

template<class M>
template<class T>
point_type_3<M>& point_type_3<M>::operator*=(T o)
{
    x *= (M)o;
    y *= (M)o;
    z *= (M)o;
    return *this;
}

template<class M>
template<class T>
point_type_3<M>& point_type_3<M>::operator/=(T o)
{
    x /= (M)o;
    y /= (M)o;
    z /= (M)o;
    return *this;
}

//

template<class M>
point_type_3<M>& point_type_3<M>::operator+=(const point_type_3<M>& o)
{
    x += o.x;
    y += o.y;
    z += o.z;
    return *this;
}

template<class M>
point_type_3<M>& point_type_3<M>::operator-=(const point_type_3<M>& o)
{
    x -= o.x;
    y -= o.y;
    z -= o.z;
    return *this;
}

template<class M>
point_type_3<M>& point_type_3<M>::operator*=(const point_type_3<M>& o)
{
    x *= o.x;
    y *= o.y;
    z *= o.z;
    return *this;
}

template<class M>
point_type_3<M>& point_type_3<M>::operator/=(const point_type_3<M>& o)
{
    x /= o.x;
    y /= o.y;
    z /= o.z;
    return *this;
}

//

template<class M>
template<class T>
point_type_3<M> point_type_3<M>::operator+(T o) const
{
    return { x + o, y + o, z + o };
}

template<class M>
template<class T>
point_type_3<M> point_type_3<M>::operator-(T o) const
{
    return { x - o, y - o, z - o };
}

template<class M>
template<class T>
point_type_3<M> point_type_3<M>::operator*(T o) const
{
    return { x * o, y * o, z * o };
}

template<class M>
template<class T>
point_type_3<M> point_type_3<M>::operator/(T o) const
{
    return { x / o, y / o, z / o };
}

//

template<class M>
point_type_3<M> point_type_3<M>::operator+(const point_type_3& o) const
{
    return { x + o.x, y + o.y, z + o.z };
}

template<class M>
point_type_3<M> point_type_3<M>::operator-(const point_type_3& o) const
{
    return { x - o.x, y - o.y, z - o.z };
}

template<class M>
point_type_3<M> point_type_3<M>::operator*(const point_type_3& o) const
{
    return { x * o.x, y * o.y, z * o.z };
}

template<class M>
point_type_3<M> point_type_3<M>::operator/(const point_type_3& o) const
{
    return { x / o.x, y / o.y, z / o.z };
}

//

template<class M>
point_type_3<M>::point_type_3()
    : x(0.0f), y(0.0f), z(0.0)
{
}

template<class M>
point_type_3<M>::point_type_3(const point_type_3& o)
    : x(o.x), y(o.y), z(o.z)
{
}

template<class M>
point_type_3<M>::point_type_3(point_type_3&& o)
    : x(o.x), y(o.y), z(o.z)
{
}

template<class M>
template<class T>
point_type_3<M>::point_type_3(const point_type<T>& o)
    : x(o.x), y(o.y), z(0.0)
{
}

template<class M>
point_type_3<M>& point_type_3<M>::operator=(const point_type_3<M>& o)
{
    x = o.x;
    y = o.y;
    z = o.z;
    return *this;
}


template<class M>
point_type_3<M>& point_type_3<M>::operator=(point_type_3<M>&& o)
{
    x = o.x;
    y = o.y;
    z = o.z;
    return *this;
}

template<class M>
void point_type_3<M>::normalize()
{
    M len = length();
    x /= len;
    y /= len;
    z /= len;
}

template<class M>
point_type_3<M> point_type_3<M>::normalized() const
{
    point_type_3 p(*this);
    p.normalize();
    return p;
}

template<class M>
M point_type_3<M>::length() const
{
    return sqrt(x * x + y * y + z * z);
}

template<class M>
point_type_3<M> point_type_3<M>::abs() const
{
    return { std::abs(x), std::abs(y), std::abs(z) };
}

template<class M>
point_type_3<M> point_type_3<M>::sign() const
{
    return { x < M(0.0) ? M(-1.0) : M(1.0), y < M(0.0) ? M(-1.0) : M(1.0), z < M(0.0) ? M(-1.0) : M(1.0) };
}

template<class M>
M point_type_3<M>::length_sqr() const
{
    return x * x + y * y + z * z;
}

template<class M>
M point_type_3<M>::angle(const point_type_3& o) const
{
    return std::acos(dot(o) / (length() * o.length()));
}

template<class M>
point_type_3<M> point_type_3<M>::cross(const point_type_3& o) const
{
    point_type_3 result;

    result.x = y * o.z - z * o.y;
    result.y = z * o.x - x * o.z;
    result.z = x * o.y - y * o.x;

    return result;
}

template<class M>
M point_type_3<M>::dot(const point_type_3& o) const
{
    return x * o.x + y * o.y + z * o.z;
}

// https://github.com/Karth42/SimpleKeplerOrbits/blob/a16ded0a21363ad3a34cb9b9168f52a96f1672aa/Assets/SimpleKeplerOrbits/Scripts/Utils/KeplerOrbitUtils.cs#L39
template<class M>
point_type_3<M> point_type_3<M>::rotated(M radians, const point_type_3<M>& normal) const
{
    double cosT = cos(radians);
    double sinT = sin(radians);
    double oneMinusCos = 1.0 - cosT;

    double a11 = oneMinusCos * normal.x * normal.x + cosT;
    double a12 = oneMinusCos * normal.x * normal.y - normal.z * sinT;
    double a13 = oneMinusCos * normal.x * normal.z + normal.y * sinT;
    double a21 = oneMinusCos * normal.x * normal.y + normal.z * sinT;
    double a22 = oneMinusCos * normal.y * normal.y + cosT;
    double a23 = oneMinusCos * normal.y * normal.z - normal.x * sinT;
    double a31 = oneMinusCos * normal.x * normal.z - normal.y * sinT;
    double a32 = oneMinusCos * normal.y * normal.z + normal.x * sinT;
    double a33 = oneMinusCos * normal.z * normal.z + cosT;

    return { x * a11 + y * a12 + z * a13,
             x * a21 + y * a22 + z * a23,
             x * a31 + y * a32 + z * a33 };
}
