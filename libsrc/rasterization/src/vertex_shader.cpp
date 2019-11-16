#include "vertex_shader.hpp"

#include "common/logger.hpp"
#include <iostream>

namespace plane_render {

VertexShader::VertexShader(const VeticesVector& vertices, const RenderingGeometryConstPtr& geom) :
    geom_(geom),
    vecs4d_src_(4, vertices.size()),
    vecs3d_transformed_(3, vertices.size()),
    pixel_points_(2, vertices.size())
{
    DCHECK(geom_);
    for (size_t i = 0; i < vertices.size(); i++)
    {
        const auto& vert_pos = vertices[i].vreg.position;
        vecs4d_src_.col(i) << vert_pos.x, vert_pos.y, vert_pos.z, 1.0;
    }
    
    Update();
}

void VertexShader::Update()
{
    std::pair<ScreenDimension, ScreenDimension> bounds = geom_->TransformGeometry(
                                                            vecs4d_src_, vecs3d_transformed_, pixel_points_);
    lower_bound_ = bounds.first;
    upper_bound_ = bounds.second;
}

} // namespace plane_render