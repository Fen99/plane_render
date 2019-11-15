#pragma once

#include "rasterization/scene_object.hpp"
#include "rasterization/graphics_types.hpp"

#include "common/logger.hpp"

#include <utility>
#include <memory>

namespace plane_render {

class Rasterizer
{
public:
    Rasterizer(const RenderingGeometryConstPtr& geom);
    Rasterizer(const Rasterizer&) = delete;
    Rasterizer& operator=(const Rasterizer&) = delete;

    // Рисует объект на сцене, используя его шейдеры
    // Пробегает по всем треугольникам, но рисует от min_line до max_line (включительно)
    void Rasterize(const SceneObject& object, ScreenDimension min_line, ScreenDimension max_line);
    void Clear();

    const Color* GetPixels() const { return pixels_; }
    size_t GetBufferSize()   const { return geom_->PixelsCount()*sizeof(Color); }

    ~Rasterizer();

private:
    bool ValidateIndex(PixelPoint p) const;
    size_t CalcIndex(PixelPoint p) const;

    Color& At(const PixelPoint& point);

    void RasterizeTriangle(const SceneObject& obj, ScreenDimension min_line, ScreenDimension max_line,
                           const TriangleIndices& indices);

private:
    static BaricentricCoords ToScreenBaricentric(const PixelPoint& p, const PixelPoint& A, const PixelPoint& B,
                                                 const PixelPoint& c);
    // "Истинные" БЦ-координаты в мировом пространстве
    static BaricentricCoords ToWorldBaricentric(const BaricentricCoords& screen_bc, float z1, float z2, float z3);

private:
    RenderingGeometryConstPtr geom_;

    float* z_buffer_ = nullptr;
    Color* pixels_ = nullptr;
};

} // namespace plane_render