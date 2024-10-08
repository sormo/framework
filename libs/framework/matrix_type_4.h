#pragma once
#include <array>
#include <point_type.h>
#include <matrix_type_3.h>
#include "HandmadeMath.h"

// HMM matrix representation 
// Elements[row][col]
// sx .  .  .
// .  sy .  .
// .  .  sz .
// tx tx tz .

class matrix_type_4
{
public:
    matrix_type_4() noexcept {}
    matrix_type_4(HMM_Mat4&& mat) noexcept : data(std::move(mat)) {}
    matrix_type_4(matrix_type_4&& mat) noexcept : data(std::move(mat.data)) {}
    matrix_type_4(const matrix_type_4& mat) noexcept : data(mat.data) {}
    matrix_type_4(const matrix_type_3<float>& mat)
    {
        data = HMM_M4D(1.0f);

        data.Elements[0][0] = mat.data[0];
        data.Elements[0][1] = mat.data[3];
        data.Elements[0][3] = mat.data[6];

        data.Elements[1][0] = mat.data[1];
        data.Elements[1][1] = mat.data[4];
        data.Elements[1][3] = mat.data[7];

        data.Elements[3][0] = mat.data[2];
        data.Elements[3][1] = mat.data[5];
        data.Elements[3][3] = mat.data[8];
    }

    static matrix_type_4 translation(float x, float y)
    {
        return HMM_Translate({ x, y, 0.0f });
    }
    static matrix_type_4 translation(float x, float y, float z)
    {
        return HMM_Translate({ x, y, z });
    }
    static matrix_type_4 translation(const point_type<float>& translation)
    {
        return HMM_Translate({ translation.x, translation.y, 0.0f });
    }
    static matrix_type_4 translation(const point_type_3<float>& translation)
    {
        return HMM_Translate({ translation.x, translation.y, translation.z });
    }
    static matrix_type_4 rotation(float radians)
    {
        return HMM_Rotate_RH(radians, { 0.0f, 0.0f, 1.0f });
    }
    static matrix_type_4 rotation(const point_type_3<float>& axis, float radians)
    {
        return HMM_Rotate_RH(radians, { axis.x, axis.y, axis.z });
    }
    static matrix_type_4 rotation(const point_type<float>& center, float radians)
    {
        auto T_origin = translation(-center);
        auto R = rotation(radians);
        auto T_center = translation(center);

        return T_center * R * T_origin;
    }
    static matrix_type_4 look_at(const point_type_3<float>& direction, const point_type_3<float>& up_direction = { 0.0f, 1.0f, 0.0f })
    {
        // TODO why not to use builtin HMM look_at ?
        // 
        // special case - no orientation transformation needed
        if (direction == up_direction)
            return HMM_M4D(1.0f);

        auto yaxis = direction;

        auto zaxis = direction.cross(up_direction);
        zaxis.normalize();

        auto xaxis = direction.cross(zaxis);
        xaxis.normalize();

        HMM_Mat4 model = HMM_M4D(1.0f);

        model.Columns[0].X = xaxis.x;
        model.Columns[0].Y = xaxis.y;
        model.Columns[0].Z = xaxis.z;

        model.Columns[1].X = yaxis.x;
        model.Columns[1].Y = yaxis.y;
        model.Columns[1].Z = yaxis.z;

        model.Columns[2].X = zaxis.x;
        model.Columns[2].Y = zaxis.y;
        model.Columns[2].Z = zaxis.z;

        return model;
    }
    static matrix_type_4 scaling(float x, float y)
    {
        return HMM_Scale({ x, y, 1.0f });
    }
    static matrix_type_4 scaling(float x, float y, float z)
    {
        return HMM_Scale({ x, y, z });
    }
    static matrix_type_4 scaling(const point_type<float>& scale)
    {
        return HMM_Scale({ scale.x, scale.y, 1.0f });
    }
    static matrix_type_4 scaling(const point_type_3<float>& scale)
    {
        return HMM_Scale({ scale.x, scale.y, scale.z });
    }
    static matrix_type_4 transform(const point_type<float>& position, float rotation, const point_type<float>& size)
    {
        auto scale = HMM_Scale({ size.x, size.y, 1.0f });
        auto rotate = HMM_Rotate_RH(rotation, HMM_Vec3{ 0.0f, 0.0f, 1.0f });
        auto translate = HMM_Translate({ position.x, position.y, 0.0f });
        return HMM_MulM4(HMM_MulM4(translate, rotate), scale);
    }
    static matrix_type_4 transform(const point_type_3<float>& position, float rotation, const point_type_3<float>& size)
    {
        auto scale = HMM_Scale({ size.x, size.y, size.z });
        auto rotate = HMM_Rotate_RH(rotation, HMM_Vec3{ 0.0f, 0.0f, 1.0f });
        auto translate = HMM_Translate({ position.x, position.y, position.z });
        return HMM_MulM4(HMM_MulM4(translate, rotate), scale);
    }
    static matrix_type_4 identity()
    {
        return HMM_M4D(1.0f);
    }
    static matrix_type_4 orthographic(float left, float right, float bottom, float top, float nearz, float farz)
    {
        return HMM_Orthographic_RH_NO(left, right, bottom, top, nearz, farz);
    }
    static matrix_type_4 perspective(float fov, float aspect_ratio, float nearz, float farz)
    {
        return HMM_Perspective_RH_NO(fov, aspect_ratio, nearz, farz);
    }

    matrix_type_4& translate(float x, float y)
    {
        data.Elements[3][0] += x;
        data.Elements[3][1] += y;
        return *this;
    }
    matrix_type_4& translate(float x, float y, float z)
    {
        data.Elements[3][0] += x;
        data.Elements[3][1] += y;
        data.Elements[3][2] += z;
        return *this;
    }
    matrix_type_4& translate(const point_type<float>& translate)
    {
        data.Elements[3][0] += translate.x;
        data.Elements[3][1] += translate.y;
        return *this;
    }
    matrix_type_4& translate(const point_type_3<float>& translate)
    {
        data.Elements[3][0] += translate.x;
        data.Elements[3][1] += translate.y;
        data.Elements[3][2] += translate.z;
        return *this;
    }
    matrix_type_4 translated(float x, float y) const
    {
        matrix_type_4 result = *this;
        result.translate(x, y);
        return result;
    }
    matrix_type_4 translated(float x, float y, float z) const
    {
        matrix_type_4 result = *this;
        result.translate(x, y, z);
        return result;
    }
    matrix_type_4 translated(const point_type<float>& translate) const
    {
        matrix_type_4 result = *this;
        result.translate(translate.x, translate.y);
        return result;
    }
    matrix_type_4 translated(const point_type_3<float>& translate) const
    {
        matrix_type_4 result = *this;
        result.translate(translate.x, translate.y, translate.z);
        return result;
    }
    matrix_type_4& rotate(float radians)
    {
        HMM_MulM4(this->data, rotation(radians).data);
        return *this;
    }
    matrix_type_4& rotate(float radians, const point_type_3<float>& axis)
    {
        HMM_MulM4(this->data, rotation(axis, radians).data);
        return *this;
    }
    matrix_type_4 rotated(float radians) const
    {
        matrix_type_4 result = *this;
        result.rotate(radians);
        return result;
    }
    matrix_type_4 rotated(float radians, const point_type_3<float>& axis) const
    {
        matrix_type_4 result = *this;
        result.rotate(radians, axis);
        return result;
    }

    matrix_type_4& scale(float x, float y)
    {
        data.Elements[0][0] *= x;
        data.Elements[1][1] *= y;
        return *this;
    }
    matrix_type_4& scale(float x, float y, float z)
    {
        data.Elements[0][0] *= x;
        data.Elements[1][1] *= y;
        data.Elements[2][2] *= z;
        return *this;
    }
    matrix_type_4& scale(const point_type<float>& s)
    {
        data.Elements[0][0] *= s.x;
        data.Elements[1][1] *= s.y;
        return *this;
    }
    matrix_type_4& scale(const point_type_3<float>& s)
    {
        data.Elements[0][0] *= s.x;
        data.Elements[1][1] *= s.y;
        data.Elements[2][2] *= s.z;
        return *this;
    }
    matrix_type_4 scaled(float x, float y) const
    {
        matrix_type_4 result = *this;
        result.scale(x, y);
        return result;
    }
    matrix_type_4 scaled(float x, float y, float z) const
    {
        matrix_type_4 result = *this;
        result.scale(x, y, z);
        return result;
    }
    matrix_type_4 scaled(const point_type<float>& s) const
    {
        matrix_type_4 result = *this;
        result.scale(s.x, s.y);
        return result;
    }
    matrix_type_4 scaled(const point_type_3<float>& s) const
    {
        matrix_type_4 result = *this;
        result.scale(s.x, s.y, s.z);
        return result;
    }

    point_type<float> get_scale() const
    {
        return { data.Elements[0][0], data.Elements[1][1] };
    }
    point_type_3<float> get_scale_3() const
    {
        return { data.Elements[0][0], data.Elements[1][1], data.Elements[2][2] };
    }
    point_type<float> get_translation() const
    {
        return { data.Elements[3][0], data.Elements[3][1] };
    }
    point_type_3<float> get_translation_3() const
    {
        return { data.Elements[3][0], data.Elements[3][1], data.Elements[3][2] };
    }
    void set_scale(float x, float y)
    {
        data.Elements[0][0] = x;
        data.Elements[1][1] = y;
    }
    void set_scale(float x, float y, float z)
    {
        data.Elements[0][0] = x;
        data.Elements[1][1] = y;
        data.Elements[2][2] = z;
    }
    void set_scale(const point_type<float>& scale)
    {
        data.Elements[0][0] = scale.x;
        data.Elements[1][1] = scale.y;
    }
    void set_scale(const point_type_3<float>& scale)
    {
        data.Elements[0][0] = scale.x;
        data.Elements[1][1] = scale.y;
        data.Elements[2][2] = scale.z;
    }
    void set_translation(float x, float y)
    {
        data.Elements[3][0] = x;
        data.Elements[3][1] = y;
    }
    void set_translation(float x, float y, float z)
    {
        data.Elements[3][0] = x;
        data.Elements[3][1] = y;
        data.Elements[3][2] = z;
    }
    void set_translation(const point_type<float>& translation)
    {
        data.Elements[3][0] = translation.x;
        data.Elements[3][1] = translation.y;
    }
    void set_translation(const point_type_3<float>& translation)
    {
        data.Elements[3][0] = translation.x;
        data.Elements[3][1] = translation.y;
        data.Elements[3][2] = translation.z;
    }

    std::array<std::array<bool, 4>, 4> operator<(const matrix_type_4& other) const
    {
        std::array<std::array<bool, 4>, 4> result;
        for (size_t i = 0; i < 4; i++)
            for (size_t j = 0; j < 4; j++)
                result[i][j] = data.Elements[i][j] < other.data.Elements[i][j];
        return result;
    }
    std::array<std::array<bool, 4>, 4> operator>(const matrix_type_4& other) const
    {
        std::array<std::array<bool, 4>, 4> result;
        for (size_t i = 0; i < 4; i++)
            for (size_t j = 0; j < 4; j++)
                result[i][j] = data.Elements[i][j] > other.data.Elements[i][j];
        return result;
    }
    std::array<std::array<bool, 4>, 4> operator==(const matrix_type_4& other) const
    {
        std::array<std::array<bool, 4>, 4> result;
        for (size_t i = 0; i < 4; i++)
            for (size_t j = 0; j < 4; j++)
                result[i][j] = data.Elements[i][j] == other.data.Elements[i][j];
        return result;
    }

    matrix_type_4& operator=(matrix_type_4&& other)
    {
        data = std::move(other.data);
        return *this;
    }

    matrix_type_4& operator=(HMM_Mat4&& other)
    {
        data = std::move(other);
        return *this;
    }

    matrix_type_4& operator=(const matrix_type_4& other)
    {
        data = other.data;
        return *this;
    }

    matrix_type_4& operator=(const HMM_Mat4& other)
    {
        data = other;
        return *this;
    }

    matrix_type_4& operator+=(float o)
    {
        for (size_t i = 0; i < 4; i++)
            for (size_t j = 0; j < 3; j++) // keep the most right column intact ?
                data.Elements[i][j] += o;
        return *this;
    }
    matrix_type_4& operator-=(float o)
    {
        for (size_t i = 0; i < 4; i++)
            for (size_t j = 0; j < 3; j++) // keep the most right column intact ?
                data.Elements[i][j] -= o;
        return *this;
    }
    matrix_type_4& operator*=(float o)
    {
        data = HMM_MulM4F(data, o);
        return *this;
    }
    matrix_type_4& operator/=(float o)
    {
        data = HMM_DivM4F(data, o);
        return *this;
    }
    matrix_type_4& operator+=(const matrix_type_4& o)
    {
        data = HMM_AddM4(data, o.data);
        return *this;
    }
    matrix_type_4& operator-=(const matrix_type_4& o)
    {
        data = HMM_SubM4(data, o.data);
        return *this;
    }
    matrix_type_4& operator*=(const matrix_type_4& o)
    {
        data = HMM_MulM4(data, o.data);
        return *this;
    }
    //matrix_type_4& operator/=(const matrix_type_4& o)
    //{
    //    HMM_Div(data, o.data);
    //    return *this;
    //}
    template<class T> matrix_type_4 operator+(T o) const
    {
        matrix_type_4 result = *this;
        result += o;
        return result;
    }
    template<class T> matrix_type_4 operator-(T o) const
    {
        matrix_type_4 result = *this;
        result -= o;
        return result;
    }
    template<class T> matrix_type_4 operator*(T o) const
    {
        matrix_type_4 result = *this;
        result *= o;
        return result;
    }
    template<class T> matrix_type_4 operator/(T o) const
    {
        matrix_type_4 result = *this;
        result /= o;
        return result;
    }
    matrix_type_4 operator+(const matrix_type_4& o) const
    {
        matrix_type_4 result = *this;
        result += o;
        return result;
    }
    matrix_type_4& operator+(const matrix_type_4& o)
    {
        *this += o;
        return *this;
    }
    matrix_type_4 operator-(const matrix_type_4& o) const
    {
        matrix_type_4 result = *this;
        result -= o;
        return result;
    }
    matrix_type_4& operator-(const matrix_type_4& o)
    {
        *this -= o;
        return *this;
    }
    matrix_type_4 operator*(const matrix_type_4& o) const
    {
        matrix_type_4 result = *this;
        result *= o;
        return result;
    }
    matrix_type_4& operator*(const matrix_type_4& o)
    {
        *this *= o;
        return *this;
    }
    //matrix_type_4 operator/(const matrix_type_4& o) const
    //{
    //    matrix_type_4 result = *this;
    //    result /= o;
    //    return result;
    //}

    matrix_type_4 transposed() const
    {
        return HMM_Transpose(data);
    }
    matrix_type_4 inverted() const
    {
        return HMM_InvGeneral(data);
    }

    template<class T> point_type<T> transform_vector(const point_type<T>& vec) const
    {
        auto res = HMM_MulM4V4(data, { vec.x, vec.y, 0.0f, 0.0f });
        return { res.X, res.Y };
    }
    template<class T> point_type_3<T> transform_vector(const point_type_3<T>& vec) const
    {
        auto res = HMM_MulM4V4(data, { vec.x, vec.y, vec.z, 0.0f });
        return { res.X, res.Y, res.Z };
    }
    template<class T> point_type<T> transform_point(const point_type<T>& vec) const
    {
        auto res = HMM_MulM4V4(data, { vec.x, vec.y, 0.0f, 1.0f });
        return { res.X, res.Y };
    }
    template<class T> point_type_3<T> transform_point(const point_type_3<T>& vec) const
    {
        auto res = HMM_MulM4V4(data, { vec.x, vec.y, vec.z, 1.0f });
        return { res.X, res.Y, res.Z };
    }

    float determinant() const
    {
        return HMM_DeterminantM4(data);
    }

    operator HMM_Mat4&&() &&
    {
        return std::move(data);
    }

    operator HMM_Mat4&() &
    {
        return data;
    }

    //operator const HMM_Mat4&() const &
    //{
    //    return data;
    //}

    HMM_Mat4 data;
};
