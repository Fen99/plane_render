#pragma once

#include "logger.hpp"
#include "aligned_allocator.hpp"

#include <cmath>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>

#define DCHECK_ALIGNMENT_16 DCHECK(((size_t) this) % 16 == 0)

namespace plane_render {

struct Vector3D
{
public:
    float x = 0;
    float y = 0;
    float z = 0;

public:
    // Медленный метод! Может использоваться, например, при загрузке модели
    Vector3D Normalized()
    {
        float norm = std::sqrt(x*x + y*y + z*z);
        return { x/norm, y/norm, z/norm };
    }
};

struct alignas(16) Vector4D // union внутри, чтобы можно было наследоваться
{
public:
    union
    {
        struct
        {
            // Возможно, правильнее было бы начать с fourth - к нему был бы простой доступ (младший).
            // Но это потребует заполнять матрицы в обратном порядке, что очень неприятно
            float x; // Младший, 0
            float y; // 1
            float z; // 2
            float fourth; // Старший, 3
        };

        float vals[4]; // Для прохождения циклом
        __m128 v4;
    };

public:
    Vector4D() { DCHECK_ALIGNMENT_16; }
    Vector4D(float x, float y, float z, float fourth) : x(x), y(y), z(z), fourth(fourth) { DCHECK_ALIGNMENT_16; }
    Vector4D(const Vector3D& vec, float fourth) : x(vec.x), y(vec.y), z(vec.z), fourth(fourth) { DCHECK_ALIGNMENT_16; }

    Vector4D(__m128 val) : v4(val) {}
    operator __m128() const { return v4; }

    inline Vector4D operator+(const Vector4D& vec) const
    {
        return _mm_add_ps(v4, vec.v4);
    }

    inline Vector4D operator-(const Vector4D& vec) const
    {
        return _mm_sub_ps(v4, vec.v4);
    }

    inline Vector4D operator*(float val) const
    {
        return _mm_mul_ps(v4, _mm_set1_ps(val));
    }
};

struct alignas(16) FastVector3D : public Vector4D // Занимает 4 байта!
{
public:
    // ПРоверка выравнивания будет в Vector4D
    FastVector3D() { }
    FastVector3D(float x, float y, float z) : Vector4D(x, y, z, 0.f) {}
    FastVector3D(const Vector3D& vec) : FastVector3D(vec.x, vec.y, vec.z) {}

    // Для облегчения преобразований
    FastVector3D(const Vector4D& vec) : Vector4D(vec) {} // Фактически, конструктор копироваия
    FastVector3D(__m128 val) : Vector4D(val) {}

    Vector3D ToVector3D() const { return { x, y, z }; }

public:
    inline float Dot(const FastVector3D& another) const
    {
        DCHECK_ALIGNMENT_16;

        // 0x71 - не перемножаем старший бит и сохраняем только в младший
        return _mm_cvtss_f32(_mm_dp_ps(v4, another.v4, 0x71));
    }

    inline FastVector3D Cross(const FastVector3D& another) const
    {
        DCHECK_ALIGNMENT_16;

        // http://threadlocalmutex.com/?p=8
        // cross(a, b).zxy = a * b.yzx - a.yzx * b; => cross(a, b) = (a * b.yzx - a.yzx * b).yzx;
        __m128 a_yzx = _mm_shuffle_ps(v4, v4, _MM_SHUFFLE(3, 0, 2, 1));
	    __m128 b_yzx = _mm_shuffle_ps(another.v4, another.v4, _MM_SHUFFLE(3, 0, 2, 1));
	    __m128 c = _mm_sub_ps(_mm_mul_ps(v4, b_yzx), _mm_mul_ps(a_yzx, another.v4));
	    return _mm_shuffle_ps(c, c, _MM_SHUFFLE(3, 0, 2, 1));
    }

    // Действует на все 4 компоненты (forth' = forth / norm, norm = sqrt(x*x+y*y+z*z))
    inline FastVector3D Normalized() const
    {
        DCHECK_ALIGNMENT_16;

        __m128 norm_bc = _mm_sqrt_ss(_mm_dp_ps(v4, v4, 0x71)); // Корень из нормы (не берем fourth)
        return _mm_div_ps(v4, _mm_broadcastss_ps(norm_bc));
    }

    inline float NormSq() const
    {
        DCHECK_ALIGNMENT_16;
        return Dot(*this);
    }
};

typedef VectorAlignment16<Vector4D> Vec4DynamicArray;
typedef VectorAlignment16<FastVector3D> FastVec3DynamicArray;

//---------------------------------------------------------------------------------------

struct alignas(16) Matrix4
{
public:
    union
    {
        float coeffitients[4][4];
        Vector4D rows[4] = {}; // Для конструкторов
    };

public:
    // Умножение на вектор-столбец
    inline Vector4D operator*(const Vector4D& vec) const
    {
        DCHECK_ALIGNMENT_16; // Т.к. нет конструктора
        Vector4D result;
        for (size_t i = 0; i < 4; i++)
            result.vals[i] = _mm_cvtss_f32(_mm_dp_ps(rows[i], vec.v4, 0xF1)); // Перемножаем все, сохраняем в младший
        return result;
    }

    inline Matrix4 operator*(const Matrix4& m) const
    {
        DCHECK_ALIGNMENT_16;
        Matrix4 result = {};
        for (size_t i = 0; i < 4; i++)
         for (size_t j = 0; j < 4; j++)
             result.rows[i].vals[j] = _mm_cvtss_f32(_mm_dp_ps(rows[i],
                                                              _mm_set_ps(m.coeffitients[3][j], m.coeffitients[2][j],
                                                                         m.coeffitients[1][j], m.coeffitients[0][j]), 0xF1));
        return result;
    }

public:
    static Matrix4 Identity()
    {
        return { 1, 0, 0, 0,
                 0, 1, 0, 0,
                 0, 0, 1, 0,
                 0, 0, 0, 1  };
    }
};

//---------------------------------------------------------------------------------------

template<typename T>
T Clump(T val, T min, T max)
{
    if (val > max)
        return max;
    if (val < min)
        return min;
    return val;
}

// Возведение в положительные степени
inline float Pow(float val, int p)
{
    float result = 1.f;
    for (int i = 0; i < p; i++)
        result *= val;
    return result;
}

} // namespace plane_render