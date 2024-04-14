#include <array>
#include <point_type.h>

template<typename M>
class matrix_type
{
public:
    static matrix_type translation(M x, M y);
    static matrix_type translation(const point_type<M>& translation);
    static matrix_type rotation(M radians);
    static matrix_type scaling(M x, M y);
    static matrix_type scaling(const point_type<M>& scale);
    static matrix_type identity();
    static matrix_type projection(const point_type<M>& size);
    static matrix_type projection(const point_type<M>& bottom_left, const point_type<M>& upper_right);

    matrix_type& translate(M x, M y);
    matrix_type& translate(const point_type<M>& scale);
    matrix_type translated(M x, M y) const;
    matrix_type translated(const point_type<M>& scale) const;

    matrix_type& rotate(M x);
    matrix_type rotated(M x) const;

    matrix_type& scale(M x, M y);
    matrix_type& scale(const point_type<M>& scale);
    matrix_type scaled(M x, M y) const;
    matrix_type scaled(const point_type<M>& scale) const;

    point_type<M> get_scale() const;
    point_type<M> get_translation() const;
    void set_scale(const point_type<M>& scale);
    void set_translation(const point_type<M>& translation);

    std::array<bool, 9> operator<(const matrix_type& other) const;
    std::array<bool, 9> operator>(const matrix_type& other) const;
    std::array<bool, 9> operator==(const matrix_type& other) const;

    template<class T> matrix_type& operator+=(T o);
    template<class T> matrix_type& operator-=(T o);
    template<class T> matrix_type& operator*=(T o);
    template<class T> matrix_type& operator/=(T o);
    matrix_type& operator+=(const matrix_type& o);
    matrix_type& operator-=(const matrix_type& o);
    //matrix_type& operator*=(const matrix_type& o);
    //matrix_type& operator/=(const matrix_type& o);
    template<class T> matrix_type operator+(T o) const;
    template<class T> matrix_type operator-(T o) const;
    template<class T> matrix_type operator*(T o) const;
    template<class T> matrix_type operator/(T o) const;
    matrix_type operator+(const matrix_type& o) const;
    matrix_type operator-(const matrix_type& o) const;
    matrix_type operator*(const matrix_type& o) const;
    //matrix_type operator/(const matrix_type& o) const;

    matrix_type transposed() const;
    matrix_type inverted() const;
    matrix_type inverted_rigid() const;

    template<class T> point_type<T> transform_vector(const point_type<T>& vector) const;
    template<class T> point_type<T> transform_point(const point_type<T>& vector) const;

    // internal
    bool is_rigid() const;
    M determinant() const;

	std::array<M, 9> data;
};

// implementation of template methods

template<typename M>
matrix_type<M> matrix_type<M>::translation(M x, M y)
{
    return { M(1), M(0),   x,
             M(0), M(1),   y,
             M(0), M(0), M(1) };
}

template<typename M>
matrix_type<M> matrix_type<M>::translation(const point_type<M>& t)
{
    return translation(t.x, t.y);
}

template<typename M>
matrix_type<M> matrix_type<M>::rotation(M radians)
{
    const M sine = std::sin(radians);
    const M cosine = std::cos(radians);

    return { cosine,   sine, M(0),
              -sine, cosine, M(0),
               M(0),   M(0), M(1) };
}

template<typename M>
matrix_type<M> matrix_type<M>::scaling(const point_type<M>& s)
{
    return scaling(s.x, s.y);
}

template<typename M>
matrix_type<M> matrix_type<M>::scaling(M x, M y)
{
    return {   x , M(0), M(0),
             M(0),   y , M(0),
             M(0), M(0), M(1) };
}

template<typename M>
matrix_type<M> matrix_type<M>::identity()
{
    return { M(1), M(0), M(0),
             M(0), M(1), M(0),
             M(0), M(0), M(1) };
}

template<typename M>
matrix_type<M> matrix_type<M>::projection(const point_type<M>& size)
{
    return scaling(size / 2.0f);
}

template<typename M>
matrix_type<M> matrix_type<M>::projection(const point_type<M>& bottom_left, const point_type<M>& upper_right)
{
    const point_type<M> difference = upper_right - bottom_left;
    const point_type<M> scale = { M(2.0) / difference.x, M(2.0) / difference.y };
    const point_type<M> offset = (upper_right + bottom_left) / difference;

    return {   scale.x,      M(0), M(0),
                  M(0),   scale.y, M(0),
             -offset.x, -offset.y, M(1) };
}

template<typename M>
matrix_type<M>& matrix_type<M>::translate(M x, M y)
{
    // TODO in place ?
    //data = { data[0], data[1], x*data[0] + y*data[1] + data[2],
    //         data[3], data[4], x*data[3] + y*data[4] + data[5],
    //         data[6], data[7], x*data[6] + y*data[7] + data[8] };

    data = (translation(x, y) * *this).data;
    return *this;
}

template<typename M>
matrix_type<M>& matrix_type<M>::translate(const point_type<M>& vec)
{
    return translate(vec.x, vec.y);
}

template<typename M>
matrix_type<M> matrix_type<M>::translated(M x, M y) const
{
    return translation(x, y) * *this;
}

template<typename M>
matrix_type<M> matrix_type<M>::translated(const point_type<M>& vec) const
{
    return translated(vec.x, vec.y);
}

template<typename M>
matrix_type<M>& matrix_type<M>:: rotate(M x)
{
    // TODO in place ?
    data = (rotation(x) * *this).data;
    return *this;
}

template<typename M>
matrix_type<M> matrix_type<M>::rotated(M x) const
{
    return rotation(x) * *this;
}

template<typename M>
matrix_type<M>& matrix_type<M>:: scale(M x, M y)
{
    data = (scaling(x, y) * *this).data;
    return *this;
}

template<typename M>
matrix_type<M>& matrix_type<M>:: scale(const point_type<M>& scale)
{
    return scale(scale.x, scale.y);
}

template<typename M>
matrix_type<M> matrix_type<M>::scaled(M x, M y) const
{
    return scaling(x, y) * *this;
}

template<typename M>
matrix_type<M> matrix_type<M>::scaled(const point_type<M>& scale) const
{
    return scaled(scale.x, scale.y);
}

template<typename M>
std::array<bool, 9> matrix_type<M>::operator<(const matrix_type& other) const
{
    std::array<bool, 9> result;
    for (int i = 0; i < 9; i++)
        result[i] = data[i] < other.data[i];
    return result;
}

template<typename M>
std::array<bool, 9> matrix_type<M>::operator>(const matrix_type& other) const
{
    std::array<bool, 9> result;
    for (int i = 0; i < 9; i++)
        result[i] = data[i] > other.data[i];
    return result;
}

template<typename M>
std::array<bool, 9> matrix_type<M>::operator==(const matrix_type& other) const
{
    std::array<bool, 9> result;
    for (int i = 0; i < 9; i++)
        result[i] = data[i] == other.data[i];
    return result;
}

template<typename M>
template<typename T>
matrix_type<M>& matrix_type<M>::operator+=(T o)
{
    for (int i = 0; i < 9; i++)
        data[i] += (M)o;
}

template<typename M>
template<typename T>
matrix_type<M>& matrix_type<M>::operator-=(T o)
{
    for (int i = 0; i < 9; i++)
        data[i] -= (M)o;
}

template<typename M>
template<typename T>
matrix_type<M>& matrix_type<M>::operator*=(T o)
{
    for (int i = 0; i < 9; i++)
        data[i] *= (M)o;
}

template<typename M>
template<typename T>
matrix_type<M>& matrix_type<M>::operator/=(T o)
{
    for (int i = 0; i < 9; i++)
        data[i] /= (M)o;
}

template<typename M>
matrix_type<M>& matrix_type<M>::operator+=(const matrix_type& o)
{
    for (int i = 0; i < 9; i++)
        data[i] += o.data[i];
}

template<typename M>
matrix_type<M>& matrix_type<M>::operator-=(const matrix_type& o)
{
    for (int i = 0; i < 9; i++)
        data[i] -= o.data[i];
}

template<typename M>
template<typename T>
matrix_type<M> matrix_type<M>::operator+(T o) const
{
    matrix_type<M> result(this->data);
    result += o;
    return result;
}

template<typename M>
template<typename T>
matrix_type<M> matrix_type<M>::operator-(T o) const
{
    matrix_type<M> result(this->data);
    result -= o;
    return result;
}

template<typename M>
template<typename T>
matrix_type<M> matrix_type<M>::operator*(T o) const
{
    matrix_type<M> result(this->data);
    result *= o;
    return result;
}

template<typename M>
template<typename T>
matrix_type<M> matrix_type<M>::operator/(T o) const
{
    matrix_type<M> result(this->data);
    result /= o;
    return result;
}

template<typename M>
matrix_type<M> matrix_type<M>::operator+(const matrix_type& o) const
{
    matrix_type<M> result(this->data);
    result += o;
    return result;
}

template<typename M>
matrix_type<M> matrix_type<M>::operator-(const matrix_type& o) const
{
    matrix_type<M> result(this->data);
    result -= o;
    return result;
}

template<typename M>
matrix_type<M> matrix_type<M>::operator*(const matrix_type& o) const
{
    matrix_type<M> result{};
    for (int r = 0; r < 3; ++r)
        for (int c = 0; c < 3; ++c)
            for (int k = 0; k < 3; ++k)
                result.data[r * 3 + c] += data[r * 3 + k] * o.data[k * 3 + c];
    return result;
}

template<typename M>
matrix_type<M> matrix_type<M>::transposed() const
{
    matrix_type<M> result{};
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            result.data[i * 3 + j] = data[j * 3 + i];
    return result;
}

template<typename M>
M matrix_type<M>::determinant() const
{
    return data[0] * (data[4] * data[8] - data[7] * data[5]) -
           data[1] * (data[3] * data[8] - data[6] * data[5]) +
           data[2] * (data[3] * data[7] - data[6] * data[4]);
}

// https://github.com/niswegmann/small-matrix-inverse/blob/master/invert3x3_c.h
template<class M>
void invert3x3(const M* src, M* dst)
{
    M det;

    /* Compute adjoint: */

    dst[0] = +src[4] * src[8] - src[5] * src[7];
    dst[1] = -src[1] * src[8] + src[2] * src[7];
    dst[2] = +src[1] * src[5] - src[2] * src[4];
    dst[3] = -src[3] * src[8] + src[5] * src[6];
    dst[4] = +src[0] * src[8] - src[2] * src[6];
    dst[5] = -src[0] * src[5] + src[2] * src[3];
    dst[6] = +src[3] * src[7] - src[4] * src[6];
    dst[7] = -src[0] * src[7] + src[1] * src[6];
    dst[8] = +src[0] * src[4] - src[1] * src[3];

    /* Compute determinant: */

    det = src[0] * dst[0] + src[1] * dst[3] + src[2] * dst[6];

    /* Multiply adjoint with reciprocal of determinant: */

    det = 1.0f / det;

    dst[0] *= det;
    dst[1] *= det;
    dst[2] *= det;
    dst[3] *= det;
    dst[4] *= det;
    dst[5] *= det;
    dst[6] *= det;
    dst[7] *= det;
    dst[8] *= det;
}

template<typename M>
matrix_type<M> matrix_type<M>::inverted() const
{
    matrix_type<M> result;

    invert3x3(data.data(), result.data.data());

    return result;
}

template<typename M>
bool matrix_type<M>::is_rigid() const
{
    // Calculate transpose
    auto t = transposed();

    // Multiply transpose by the original matrix
    auto product = t * *this;

    // Check if the result is close to the identity matrix
    constexpr float epsilon = 1e-6;
    for (int i = 0; i < 9; ++i)
    {
        if (i % 4 == 0 && std::abs(product.data[i] - 1.0f) > epsilon)
            return false;  // Diagonal elements should be close to 1
        else if (i % 4 != 0 && std::abs(product.data[i]) > epsilon)
            return false;  // Off-diagonal elements should be close to 0
    }

    return true;
}

template<typename M>
matrix_type<M> matrix_type<M>::inverted_rigid() const
{
    // assume rigid

    // The inverse of the rotation matrix is its transpose
    matrix_type<M> result;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            result[i * 3 + j] = data[j * 3 + i];

    // The inverse of the translation vector is its negation
    result.data[6] = -data[6];
    result.data[7] = -data[7];
    result.data[8] = -data[8];

    return result;
}

template<typename M>
template<typename T>
point_type<T> matrix_type<M>::transform_vector(const point_type<T>& vector) const
{
    return { data[0] * vector.x + data[1] * vector.y,
             data[3] * vector.x + data[4] * vector.y };
}

template<typename M>
template<typename T>
point_type<T> matrix_type<M>::transform_point(const point_type<T>& vector) const
{
    return { data[0] * vector.x + data[1] * vector.y + data[2],
             data[3] * vector.x + data[4] * vector.y + data[5] };
}

template<typename M>
point_type<M> matrix_type<M>::get_scale() const
{
    return { data[0], data[4] };
}

template<typename M>
point_type<M> matrix_type<M>::get_translation() const
{
    return { data[2], data[5] };
}

template<typename M>
void matrix_type<M>::set_scale(const point_type<M>& scale)
{
    data[0] = scale.x;
    data[4] = scale.y;
}

template<typename M>
void matrix_type<M>::set_translation(const point_type<M>& translation)
{
    data[2] = translation.x;
    data[5] = translation.y;
}
