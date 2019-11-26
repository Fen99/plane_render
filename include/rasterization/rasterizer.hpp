#pragma once

#include "rasterization/scene_object.hpp"
#include "rasterization/screen_buffer.hpp"

#include <utility>
#include <memory>

namespace plane_render {

class Rasterizer
{
// Ставим сюда, чтобы inline компилировался
private:
    RenderingGeometryConstPtr geom_;
    ScreenBuffer screen_buffer_;

public:
    Rasterizer(const RenderingGeometryConstPtr& geom);
    Rasterizer(const Rasterizer&) = delete;
    Rasterizer& operator=(const Rasterizer&) = delete;

    // Растеризует треугольники объекта obj, начиная со start и в количестве count
    // Из obj берутся vertices и indices
    // Вершины имеют индексы for s in range(start, start+count): (indices[s]; indices[s+1]; indices[s+2])
    // Если start+count >= len(indices) => return
    void Rasterize(const SceneObject& obj, size_t start, size_t count);
    void Clear() { screen_buffer_.Clear(); }

    const Color* GetPixels() const { return screen_buffer_.GetPixels(); }
    size_t GetBufferSize()   const { return screen_buffer_.GetBufferSize(); }

private:
    // Вызывающий сам проверяет, что (A, B, C).z <= -GraphicsEps
    void RasterizeTriangle(const SceneObject& obj, const Vertex& A, const Vertex& B, const Vertex& C);
};

} // namespace plane_render