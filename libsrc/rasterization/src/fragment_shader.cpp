#include "fragment_shader.hpp"

#include <cmath>
#include <cassert>

namespace plane_render {

FragmentShader::FragmentShader(const RenderingGeometryConstPtr& geom) :
    geom_(geom)
{}

Color FragmentShader::ProcessFragment(const Vertex& avg_vertex) const
{
    FastVector3D light = (geom_->LightPos() - avg_vertex.vertex_coords);

    float normal_normsq = avg_vertex.normal.NormSq();
    float diff = std::max(avg_vertex.normal.Dot(light) / std::sqrt(light.NormSq() * normal_normsq), 0.f);

    FastVector3D h = light - avg_vertex.vertex_coords; // camera = (0, 0, 0) - avg_vertex.vertex_coords
    const float prod = std::max(h.Dot(avg_vertex.normal) / std::sqrt(h.NormSq() * normal_normsq), 0.f);

    // std::pow убивает производительность!
    //float spec = std::pow(prod, lightN_);  
    float spec = Pow(prod, lightN_);

    const float light_intens = 0.2f + 0.4f*diff + 0.4f*spec;
    return texture_.GetPoint(avg_vertex.texture_coords) * light_intens;
    //return { 0, (Color::ColorElement) (((int) avg_vertex.vertex_coords.z*10) % 255) };
}

void FragmentShader::LoadTexture(const std::string& name)
{
    texture_.Load(name);
}

} // namespace plane_render
