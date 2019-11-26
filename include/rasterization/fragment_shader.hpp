#pragma once

#include "rasterization/rasterizer.hpp"
#include "rasterization/texture.hpp"
#include "rasterization/graphics_types.hpp"

namespace plane_render {

class FragmentShader
{
public:
    FragmentShader(const RenderingGeometryConstPtr& info);
    FragmentShader(const FragmentShader&) = delete;
    FragmentShader& operator=(const FragmentShader&) = delete;
    virtual ~FragmentShader() {}

    void LoadTexture(const std::string& texture_name);
    const Texture* GetTexture() const { return &texture_; }

    inline float PhongLight(const Vertex& avg_vertex) const
    {
        FastVector3D light = (geom_->LightPos() - avg_vertex.vertex_coords);

        float normal_normsq = avg_vertex.normal.NormSq();
        float diff = std::max(avg_vertex.normal.Dot(light) / std::sqrt(light.NormSq() * normal_normsq), 0.f);

        FastVector3D h = light - avg_vertex.vertex_coords; // camera = (0, 0, 0) - avg_vertex.vertex_coords
        const float prod = std::max(h.Dot(avg_vertex.normal) / std::sqrt(h.NormSq() * normal_normsq), 0.f);

        // std::pow убивает производительность!
        //float spec = std::pow(prod, lightN_);
        float spec = Pow(prod, lightN_);
        return 0.2f + 0.4f*diff + 0.4f*spec;
    }

    virtual Color ProcessFragment(const Vertex& vertex_avg) const;

protected:
    RenderingGeometryConstPtr geom_;
    Texture texture_;

private:
    static constexpr size_t lightN_ = 3;
};

} // namespace plane_render