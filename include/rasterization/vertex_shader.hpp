#pragma once

#include "rasterization/graphics_types.hpp"
#include "rasterization/rendering_geometry.hpp"

#include <memory>
#include <vector>

namespace plane_render {

class VertexShader
{
public:
    virtual void Update(); // Берет информацию из RenderingInfo о движении камеры

    const Vector3D* GetPoint(size_t index) const;
    const PixelPoint* GetAsPixelPoint(size_t index) const; // Без Clump!

    ScreenDimension GetLowerRow() const { return lower_bound_; }
    ScreenDimension GetUpperRow() const { return upper_bound_; }

    VertexShader(const VeticesVector& vertices, const RenderingGeometryConstPtr& geom);
    VertexShader(const VertexShader&) = delete;
    VertexShader& operator=(const VertexShader&) = delete;
    virtual ~VertexShader() {}

private:
    RenderingGeometryConstPtr geom_;

    RenderingGeometry::SrcGeometryMatrix vecs4d_src_; // Колонки: (3d-вектора; 1.0) для вершин

    // Объект на экране
    RenderingGeometry::TransformedGeometryMatrix vecs3d_transformed_; // 3d-вектора
    RenderingGeometry::PixelsMatrix pixel_points_;

    // Для ускорения: границы линий, где может быть найден объект (т.к. рендерим по строкам)
    ScreenDimension lower_bound_ = std::numeric_limits<ScreenDimension>::max();
    ScreenDimension upper_bound_ = -1;
};

} // namespace plane_render