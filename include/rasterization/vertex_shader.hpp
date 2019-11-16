#pragma once

#include "rasterization/graphics_types.hpp"
#include "rasterization/rendering_geometry.hpp"

#include <memory>
#include <vector>

namespace plane_render {

class VertexShader
{
// Чтобы inline работал
private:
    RenderingGeometryConstPtr geom_;

    RenderingGeometry::SrcGeometryMatrix vecs4d_src_; // Колонки: (3d-вектора; 1.0) для вершин

    // Объект на экране
    RenderingGeometry::TransformedGeometryMatrix vecs3d_transformed_; // 3d-вектора
    RenderingGeometry::PixelsMatrix pixel_points_;

    // Для ускорения: границы линий, где может быть найден объект (т.к. рендерим по строкам)
    ScreenDimension lower_bound_ = std::numeric_limits<ScreenDimension>::max();
    ScreenDimension upper_bound_ = -1;

public:
    virtual void Update(); // Берет информацию из RenderingInfo о движении камеры

    inline const Vector3D* GetPoint(size_t index) const
    {
        DCHECK((int) index < vecs3d_transformed_.cols());
        return reinterpret_cast<const Vector3D*>(vecs3d_transformed_.data() + index*(sizeof(Vector3D)/sizeof(float)));
    }

    inline const PixelPoint* GetAsPixelPoint(size_t index) const // Без Clump!
    {
        DCHECK((int) index < pixel_points_.cols());
        return reinterpret_cast<const PixelPoint*>(pixel_points_.data() + index*(sizeof(PixelPoint)/sizeof(ScreenDimension)));
    }

    ScreenDimension GetLowerRow() const { return lower_bound_; }
    ScreenDimension GetUpperRow() const { return upper_bound_; }

    VertexShader(const VeticesVector& vertices, const RenderingGeometryConstPtr& geom);
    VertexShader(const VertexShader&) = delete;
    VertexShader& operator=(const VertexShader&) = delete;
    virtual ~VertexShader() {}

};

} // namespace plane_render