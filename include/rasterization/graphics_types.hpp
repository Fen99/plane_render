#pragma once

#include "common/aligned_allocator.hpp"
#include "common/math_simple.hpp"

#include <functional>
#include <utility>
#include <vector>
#include <memory>
#include <xmmintrin.h>

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
    typedef unsigned char ColorElement;

    ColorElement A = 0;
    ColorElement R = 0;
    ColorElement G = 0;
    ColorElement B = 0;
};
constexpr Color Red = { 0, 255, 0, 0 };
constexpr Color Green = { 0, 0, 255, 0 };
constexpr Color Blue = { 0, 0, 0, 255 };


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

    Vertex operator*(float val) const;
    Vertex operator+(const Vertex& another) const;
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
    bool IsValid() const;

public:
    float a = 0.f;
    float b = 0.f;
    float c = 0.f;
};

} // namespace plane_render
