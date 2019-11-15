#pragma once

namespace plane_render {

struct Vector3D
{
public:
    float Dot(const Vector3D& another) const;
    Vector3D Cross(const Vector3D& another) const;
    Vector3D Normalized() const;
    float NormSq() const;

    Vector3D operator+(const Vector3D& v2) const;
    Vector3D operator-(const Vector3D& v2) const;

public:
    float x = 0;
    float y = 0;
    float z = 0;
};

struct Vector4D
{
public:
    Vector3D ToVector3D() const;

public:
    Vector3D point;
    float fourth = 0;
};

template<typename T>
T Clump(T val, T min, T max)
{
    if (val > max)
        return max;
    if (val < min)
        return min;
    return val;
}

} // namespace plane_render