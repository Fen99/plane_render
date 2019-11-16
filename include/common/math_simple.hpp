#pragma once

#include <cmath>

namespace plane_render {

struct Vector3D
{
public:
    inline float Dot(const Vector3D& another) const
    {
        return x*another.x + y*another.y + z*another.z;
    }

    inline Vector3D Cross(const Vector3D& another) const
    {
        return { y*another.z - z*another.y, z*another.x - x*another.z, x*another.y - y*another.x };
    }

    inline Vector3D Normalized() const
    {
        float norm = std::sqrt(NormSq());
        return { x / norm, y / norm, z / norm };
    }

    inline float NormSq() const
    {
        return x*x + y*y + z*z;
    }

    inline Vector3D operator+(const Vector3D& v2) const
    {
        return { x + v2.x, y + v2.y, z + v2.z };
    }
    
    inline Vector3D operator-(const Vector3D& v2) const
    {
        return { x - v2.x, y - v2.y, z - v2.z };
    }

public:
    float x = 0;
    float y = 0;
    float z = 0;
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