#pragma once

#include "common/aligned_allocator.hpp"
#include "common/math_simple.hpp"

#include <functional>
#include <utility>
#include <vector>
#include <memory>
#include <cmath>
#include <xmmintrin.h>
#include <pmmintrin.h>

namespace plane_render {

template<typename T>
struct Point2D
{
    T x = 0;
    T y = 0;
};

// Camera rotation
struct RotationAngles
{
    float phi = 0;
    float theta = 0;
};

// World coordinates
using WorldPoint = Vector3D;  // Точка в мировом пространстве

// Screen coordinates (in pixels)
typedef int ScreenDimension;
using PixelPoint = Point2D<ScreenDimension>;

// Colors
struct Color // Order for SDL
{
public:
    typedef unsigned char ColorElement;

public:
    ColorElement A = 0;
    ColorElement R = 0;
    ColorElement G = 0;
    ColorElement B = 0;

public:
    inline Color operator*(float val) const
    {
        return { static_cast<ColorElement>(A*val),
                 static_cast<ColorElement>(R*val), 
                 static_cast<ColorElement>(G*val),
                 static_cast<ColorElement>(B*val) };
    }
};
constexpr Color Red   = { 0, 255, 0, 0 };
constexpr Color Green = { 0, 0, 255, 0 };
constexpr Color Blue  = { 0, 0, 0, 255 };

// Objects
using TextureCoords = Point2D<float>;
struct VertexProperties
{
    TextureCoords texture_coords;
    Vector3D normal;

    VertexProperties(const TextureCoords& tex, const Vector3D& norm) : texture_coords(tex), normal(norm)
    {}
};

union Vertex
{
    struct
    {
        WorldPoint position;
        VertexProperties properties;
    } vreg;

    struct // Для быстрых вычислений
    {
        __m128 v1;
        __m128 v2;
    } vfast;

    Vertex(const WorldPoint& pos, const TextureCoords& tex, const Vector3D& norm) :
        vreg{pos, {tex, norm}}
    {}
    Vertex(__m128 v1_p, __m128 v2_p) :
        vfast{v1_p, v2_p}
    {}

    inline Vertex operator*(float val) const
    {
        __m128 val_bc = _mm_set_ps1(val);
        Vertex result = { _mm_mul_ps(vfast.v1, val_bc), _mm_mul_ps(vfast.v2, val_bc) };
        return result;
         //return {{ vreg.position.x*val, vreg.position.y * val, vreg.position.z * val }, { vreg.properties.texture_coords.x * val, vreg.properties.texture_coords.y * val },
         //        { vreg.properties.normal.x * val, vreg.properties.normal.y * val, vreg.properties.normal.z * val }};
    }

    inline Vertex operator+(const Vertex& v_second) const
    {
        Vertex result = {_mm_add_ps(vfast.v1, v_second.vfast.v1), _mm_add_ps(vfast.v2, v_second.vfast.v2)};
        return result;
//         return { {vreg.position.x + vp_second.vreg.position.x, vreg.position.y + vp_second.vreg.position.y, vreg.position.z + vp_second.vreg.position.z }, 
//                  { vreg.properties.texture_coords.x + vp_second.vreg.properties.texture_coords.x, vreg.properties.texture_coords.y + vp_second.vreg.properties.texture_coords.y},
//                  { vreg.properties.normal.x + vp_second.vreg.properties.normal.x, vreg.properties.normal.y + vp_second.vreg.properties.normal.y, vreg.properties.normal.z + vp_second.vreg.properties.normal.z} };
    }
};
typedef VectorAlignment16<Vertex> VeticesVector;

// Triangles
struct TriangleIndices
{
    size_t v1 = 0;
    size_t v2 = 0;
    size_t v3 = 0;
};

// Барицентрические координаты
struct BaricentricCoords
{
public:
    float a = 0.f;
    float b = 0.f;
    float c = 0.f;

public:
    inline bool IsValid() const
    {
        return a >= 0 && b >= 0 && c >= 0;
    }
};

} // namespace plane_render
