#include "fragment_shader.hpp"

#include <cmath>
#include <cassert>

namespace plane_render {

FragmentShader::FragmentShader(const RenderingGeometryConstPtr& geom) :
    geom_(geom)
{}

Color FragmentShader::ProcessFragment(const Vertex& avg_vertex) const
{
    Vector3D light = (geom_->LightPos() - avg_vertex.vreg.position);
    Vector3D cam = (geom_->CameraPos() - avg_vertex.vreg.position);

    float diff = std::abs(cam.Dot(light) / std::sqrt(light.NormSq() * cam.NormSq()));
    Vector3D h = (light + cam);
    float prod = h.Dot(avg_vertex.vreg.properties.normal) / std::sqrt(h.NormSq() * avg_vertex.vreg.properties.normal.NormSq());
    float spec = std::pow(std::abs(prod), lightN_);

    float light_intens = Clump(0.2f + 0.3f*diff + 0.5f*spec, 0.f, 1.f);
    // FIXME! Временно, для отладки
    //if (texture_)
        return texture_.GetPoint(avg_vertex.vreg.properties.texture_coords) * light_intens;
    //else
    //    return { 0, (Color::ColorElement) (255*light_intens) };
}

void FragmentShader::LoadTexture(const std::string& name)
{
    texture_.Load(name);
}

} // namespace plane_render
