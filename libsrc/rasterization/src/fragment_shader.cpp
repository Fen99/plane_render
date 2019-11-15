#include "fragment_shader.hpp"

#include <cmath>
#include <cassert>

namespace plane_render {

FragmentShader::FragmentShader(const RenderingGeometryConstPtr& geom) :
    geom_(geom)
{}

Vertex FragmentShader::AgregateVertices(const BaricentricCoords& bc, const Vertex& v1, const Vertex& v2, const Vertex& v3)
{
    return v1*bc.a + v2*bc.b + v3*bc.c;
}

Color FragmentShader::ProcessFragment(const Vertex& avg_vertex) const
{
    Vector3D light = (geom_->LightPos() - avg_vertex.vreg.position);
    Vector3D cam = (geom_->CameraPos() - avg_vertex.vreg.position);

    float diff = std::abs(cam.Dot(light) / std::sqrt(light.NormSq() * cam.NormSq()));
    Vector3D h = (light + cam);
    float prod = h.Dot(avg_vertex.vreg.properties.normal) / std::sqrt(h.NormSq() * avg_vertex.vreg.properties.normal.NormSq());
    float spec = std::pow(std::abs(prod), lightN_);

    float light_intens = 0.2f + 0.3f*diff + 0.5f*spec;
    if (std::isinf(light_intens))
        light_intens = 0.2f;
    if (light_intens < 0)
        light_intens = 0;
    if (light_intens > 1)
        light_intens = 1;

    return { 0, (Color::ColorElement) (255*light_intens) };
}

} // namespace plane_render
