#pragma once

#include "rasterization/scene_object.hpp"
#include "rasterization/rasterizer.hpp"
#include "threadpool/threadpool.hpp"

#include <vector>

namespace plane_render {

class RasterizationPipeline
{
private:
    static constexpr size_t ThreadsCount = 8;
    static constexpr size_t TrianglesPerTask = 1000;

public:
    RasterizationPipeline(const RenderingGeometryPtr& geom, std::vector<SceneObject>&& objects_names);
    RasterizationPipeline(const RasterizationPipeline&) = delete;
    RasterizationPipeline& operator=(const RasterizationPipeline&) = delete;

    void CameraMove(float dx, float dy, float dz);
    void CameraRotate(float phi, float theta);

    // Перерисовывает экран
    void Update();

    const Color* GetPixels() const { return rasterizer_.GetPixels(); }
    size_t GetBufferSize()   const { return rasterizer_.GetBufferSize(); }
    const std::vector<SceneObject>& GetObjects() const { return objects_; }

    ScreenDimension ScreenWidth()  const { return geom_->Width();  }
    ScreenDimension ScreenHeight() const { return geom_->Height(); }

private:
    RenderingGeometryPtr geom_;
    std::vector<SceneObject> objects_;
    ThreadPool pool_;

    Rasterizer rasterizer_;

    size_t iterations_ = 0;
    std::pair<double, double> sum_time_ = { 0.0, 0.0 }; // vs; fs
};

typedef std::shared_ptr<RasterizationPipeline> RasterizationPipelinePtr;

} // namespace plane_render
