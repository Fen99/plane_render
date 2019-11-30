#pragma once

#include "rasterization/scene_object.hpp"
#include "rasterization/rasterizer.hpp"
#include "threadpool/threadpool.hpp"

#include "sdl_adapter/render_provider.hpp"

#include <vector>
#include <fstream>

namespace plane_render {

class RasterizationPipeline : public IRenderProvider
{
private:
    static constexpr size_t ThreadsCount = 8;

public:
    // perf_filename - куда писать перформанс. Формат - <total>\t<vs>\t<fs+rast>
    RasterizationPipeline(const RenderingGeometryPtr& geom, std::vector<SceneObject>&& objects,
                          const std::string& perf_filename);
    RasterizationPipeline(const RasterizationPipeline&) = delete;
    RasterizationPipeline& operator=(const RasterizationPipeline&) = delete;

    virtual void MoveCam(float dx, float dy, float dz) override;
    virtual void MoveAt (float dx, float dy, float dz) override;

    // Перерисовывает экран
    virtual void Update() override;

    virtual const Color* GetPixels() const override { return rasterizer_.GetPixels(); }
    virtual size_t GetBufferSize() const override   { return rasterizer_.GetBufferSize(); }
    const std::vector<SceneObject>& GetObjects() const { return objects_; }

    virtual ScreenDimension ScreenWidth() const override  { return geom_->Width();  }
    virtual ScreenDimension ScreenHeight() const override { return geom_->Height(); }

private:
    RenderingGeometryPtr geom_;
    std::vector<SceneObject> objects_;
    ThreadPool pool_;

    Rasterizer rasterizer_;

    std::ofstream perf_output_;
};

} // namespace plane_render
