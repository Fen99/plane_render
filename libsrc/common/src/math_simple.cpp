#include "math_simple.hpp"

#include <cmath>

namespace plane_render {

float Vector3D::Dot(const Vector3D& another) const
{
    return x*another.x + y*another.y + z*another.z;
}

Vector3D Vector3D::Cross(const Vector3D& another) const
{
    return { y*another.z - z*another.y, z*another.x - x*another.z, x*another.y - y*another.x };
}

Vector3D Vector3D::operator+(const Vector3D& v2) const
{
    return { x + v2.x, y + v2.y, z + v2.z };
}

Vector3D Vector3D::operator-(const Vector3D& v2) const
{
    return {x - v2.x, y - v2.y, z - v2.z};
}

float Vector3D::NormSq() const
{
    return x*x + y*y + z*z;
}

Vector3D Vector3D::Normalized() const
{
    float norm = std::sqrt(NormSq());
    return { x / norm, y / norm, z / norm };
}

Vector3D Vector4D::ToVector3D() const
{
    return { point.x / fourth, point.y / fourth, point.z / fourth };
}

} // namespace plane_render