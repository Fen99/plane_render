#pragma once

#include "rasterization/rasterizer.hpp"
#include "rasterization/texture.hpp"
#include "rasterization/graphics_types.hpp"

namespace plane_render {

class FragmentShader
{
public:
    FragmentShader(const RenderingGeometryConstPtr& info, int light_n = 3);
    FragmentShader(const FragmentShader&) = delete;
    FragmentShader& operator=(const FragmentShader&) = delete;
    virtual ~FragmentShader() {}

    void LoadTexture(const std::string& texture_name);
    const Texture* GetTexture() const { return &texture_; }

    virtual Color ProcessFragment(const Vertex& vertex_avg) const;

public:
    inline static Vertex AgregateVertices(const BaricentricCoords& bc, const Vertex& v1, const Vertex& v2, const Vertex& v3)
    {
        return v1*bc.a + v2*bc.b + v3*bc.c;
    }

private:
    RenderingGeometryConstPtr geom_;
    Texture texture_;

private:
    int lightN_ = 3;
};

} // namespace plane_render