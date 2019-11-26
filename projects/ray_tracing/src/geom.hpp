#pragma once
#include <cmath>
struct vec2
{
    float x, y;
};
union vec4
{
    struct
    {
        float x, y, z, w;
    };
    float data[4];

    float       &operator[](int i)       noexcept {return data[i];}
    float const &operator[](int i) const noexcept {return data[i];}
};
union vec3
{
    struct
    {
        float x, y, z;
    };
    float data[3];

    vec3() = default;
    vec3(float const a, float const b, float const c) noexcept
        : data{a, b, c}
    {}
    vec3(vec4 const &v) noexcept
        : data{v.x / v.w, v.y / v.w, v.z / v.w}
    {}

    float       &operator[](int i)       noexcept {return data[i];}
    float const &operator[](int i) const noexcept {return data[i];}
};
inline vec3 operator+(vec3 const &v1, vec3 const &v2) noexcept
{
    return vec3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}
inline vec3 operator-(vec3 const &v1, vec3 const &v2) noexcept
{
    return vec3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}
inline vec3 operator*(vec3 const &v, float const f) noexcept
{
    return vec3(v.x * f, v.y * f, v.z * f);
}
inline vec3 cross(vec3 const &v1, vec3 const &v2) noexcept
{
    return vec3(v1.y * v2.z - v1.z * v2.y, v1.z * v2.x - v1.x * v2.z, v1.x * v2.y - v1.y * v2.x);
}
inline float dot(vec3 const &v1, vec3 const &v2) noexcept
{
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}
inline float length(vec3 const &v) noexcept
{
    return std::sqrt(dot(v, v));
}
inline vec3 normalize(vec3 const &v) noexcept
{
    float const len = length(v);
    return vec3(v.x / len, v.y / len, v.z / len);
}

struct mat3
{
    float data[3][3];
    float       *operator[](int i)       noexcept {return data[i];}
    float const *operator[](int i) const noexcept {return data[i];}
};
inline vec3 operator*(mat3 const &m, vec3 const &v) noexcept
{
    vec3 out = vec3(0.f, 0.f, 0.f);
    for(int i = 0; i < 3; ++i)
        for(int j = 0; j < 3; ++j)
            out[i] += m[i][j] * v[j];
    return out;
}
inline mat3 rotate(vec3 const &dir, vec3 const &_up) noexcept
{
    vec3 const right = normalize(cross(_up, dir));
    vec3 const up = cross(dir, right);
    return mat3
    {
        right.x, right.y, right.z,
        up.x,    up.y,    up.z,
        dir.x,   dir.y,   dir.z
    };
}
