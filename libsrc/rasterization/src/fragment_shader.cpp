#include "fragment_shader.hpp"

#include <cmath>
#include <cassert>

namespace plane_render {

FragmentShader::FragmentShader(const RenderingGeometryConstPtr& geom, size_t light_n) :
    geom_(geom),
    lightN_(light_n)
{}

Color FragmentShader::ProcessFragment(const Vertex& avg_vertex) const
{
    const Vector3D light = (geom_->LightPos() - avg_vertex.vreg.position);
    const Vector3D cam = (geom_->CameraPos() - avg_vertex.vreg.position);

    const float normal_normsq = avg_vertex.vreg.properties.normal.NormSq();
    const float diff = std::max(avg_vertex.vreg.properties.normal.Dot(light) / std::sqrt(light.NormSq() * normal_normsq), 0.f);

    const Vector3D h = (light + cam);
    const float prod = std::max(h.Dot(avg_vertex.vreg.properties.normal) / std::sqrt(h.NormSq() * normal_normsq), 0.f);

    // Убивает производительность!
    //float spec = std::pow(prod, lightN_);  
    float spec = 1.f;
    for (size_t i = 0; i < lightN_; i++)
        spec *= prod;

    const float light_intens = 0.2f + 0.4f*diff + 0.4f*spec;
    return texture_.GetPoint(avg_vertex.vreg.properties.texture_coords) * light_intens;
    // return { 0, (Color::ColorElement) (255*light_intens) };
}

void FragmentShader::LoadTexture(const std::string& name)
{
    texture_.Load(name);
}

} // namespace plane_render
