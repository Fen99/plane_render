#pragma once

#include "rasterization/rasterizer.hpp"
#include "rasterization/graphics_types.hpp"

namespace plane_render {

class FragmentShader
{
public:
    FragmentShader(const RenderingGeometryConstPtr& info);
    FragmentShader(const FragmentShader&) = delete;
    FragmentShader& operator=(const FragmentShader&) = delete;
    virtual ~FragmentShader() {}

    virtual Color ProcessFragment(const Vertex& vertex_avg) const;

public:
    static Vertex AgregateVertices(const BaricentricCoords& bc, const Vertex& v1, const Vertex& v2, const Vertex& v3);

private:
    RenderingGeometryConstPtr geom_;

private:
    static constexpr int lightN_ = 50;
};

} // namespace plane_render