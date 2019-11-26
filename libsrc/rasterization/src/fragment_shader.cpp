#include "fragment_shader.hpp"

#include <cmath>
#include <cassert>

namespace plane_render {

FragmentShader::FragmentShader(const RenderingGeometryConstPtr& geom) :
    geom_(geom)
{}

Color FragmentShader::ProcessFragment(const Vertex& avg_vertex) const
{
    return texture_.GetPoint(avg_vertex.texture_coords) * PhongLight(avg_vertex);
    //return { 0, (Color::ColorElement) (((int) avg_vertex.vertex_coords.z*10) % 255) };
}

void FragmentShader::LoadTexture(const std::string& name)
{
    texture_.Load(name);
}

} // namespace plane_render
