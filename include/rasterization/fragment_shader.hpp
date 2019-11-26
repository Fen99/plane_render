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

    virtual Color ProcessFragment(const Vertex& vertex_avg) const;

private:
    RenderingGeometryConstPtr geom_;
    Texture texture_;

private:
    static constexpr size_t lightN_ = 3;
};

} // namespace plane_render