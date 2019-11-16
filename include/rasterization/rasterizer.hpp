#pragma once

#include "rasterization/scene_object.hpp"
#include "rasterization/graphics_types.hpp"

#include "common/logger.hpp"

#include <utility>
#include <memory>

namespace plane_render {

class Rasterizer
{
// Ставим сюда, чтобы inline компилировался
private:
    RenderingGeometryConstPtr geom_;

    float* z_buffer_ = nullptr;
    Color* pixels_ = nullptr;

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
    inline size_t CalcIndex(const PixelPoint& point) const
    {
        DCHECK(ValidateIndex(point));
        return point.y*geom_->Width() + point.x;
    }
    inline Color& At(const PixelPoint& point) { return pixels_[CalcIndex(point)]; }

    void RasterizeTriangle(const SceneObject& obj, ScreenDimension min_line, ScreenDimension max_line,
                           const TriangleIndices& indices);

private:
    inline bool ValidateIndex(const PixelPoint& point) const
    {
        return point.x < geom_->Width() && point.y < geom_->Height() && point.x >= 0 && point.y >= 0;
    }

    inline static BaricentricCoords ToScreenBaricentric(
        const PixelPoint& p, const PixelPoint& A, const PixelPoint& B, const PixelPoint& C)
    {
        Vector3D x_vec = { (float) B.x - A.x, (float) C.x - A.x, (float) A.x - p.x };
        Vector3D y_vec = { (float) B.y - A.y, (float) C.y - A.y, (float) A.y - p.y };
        Vector3D cross = x_vec.Cross(y_vec);
        return { 1.f - (cross.x+cross.y)/cross.z, cross.x/cross.z, cross.y/cross.z };
    }

    // "Истинные" БЦ-координаты в мировом пространстве
    inline static BaricentricCoords ToWorldBaricentric(const BaricentricCoords& screen_bc, float z1, float z2, float z3)
    {
        float alphap = screen_bc.a / z1;
        float betap  = screen_bc.b / z2;
        float gammap = screen_bc.c / z3;
        float summ = alphap + betap + gammap;

        return { alphap/summ, betap/summ, gammap/summ };
    }
};

} // namespace plane_render