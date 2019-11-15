﻿#pragma once

#include "rasterization/scene_object.hpp"
#include "rasterization/rasterizer.hpp"
#include "threadpool/threadpool.hpp"

#include <vector>

namespace plane_render {

class RasterizationPipeline
{
private:
    static constexpr size_t ThreadsCount = 8;

public:
    RasterizationPipeline(const RenderingGeometryPtr& geom, const std::vector<std::string>& objects_names);
    RasterizationPipeline(const RasterizationPipeline&) = delete;
    RasterizationPipeline& operator=(const RasterizationPipeline&) = delete;

    void CameraMove(float dx, float dy, float dz);
    void CameraRotate(float phi, float theta);

    // Перерисовывает экран
    void Update();

    const Color* GetPixels() const { return rasterizer_.GetPixels(); }
    size_t GetBufferSize()   const { return rasterizer_.GetBufferSize(); }

    ScreenDimension ScreenWidth()  const { return geom_->Width();  }
    ScreenDimension ScreenHeight() const { return geom_->Height(); }

private:
    RenderingGeometryPtr geom_;
    std::vector<SceneObject> objects_;
    ThreadPool pool_;

    Rasterizer rasterizer_;
};

typedef std::shared_ptr<RasterizationPipeline> RasterizationPipelinePtr;

} // namespace plane_render
