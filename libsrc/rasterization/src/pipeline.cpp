#include "pipeline.hpp"

#include "vertex_shader.hpp"
#include "fragment_shader.hpp"

#include <chrono>

namespace plane_render {

RasterizationPipeline::RasterizationPipeline(const RenderingGeometryPtr& geom,
                                             std::vector<SceneObject>&& objects) :
    geom_(geom),
    objects_(std::move(objects)),
    pool_(ThreadsCount),
    rasterizer_(geom_)
{}

void RasterizationPipeline::CameraMove(float dx, float dy, float dz)
{
    geom_->Move({ dx, dy, dz });
    Update();
}

void RasterizationPipeline::CameraRotate(float phi, float theta)
{
    geom_->Rotate({ phi, theta });
}

void RasterizationPipeline::Update()
{
    rasterizer_.Clear();

    auto const t0 = std::chrono::system_clock::now();
    for (auto& obj : objects_)
    {
        obj.Update();
    }

    auto tv = std::chrono::system_clock::now();
    for (auto& obj : objects_)
    {
        std::vector<ThreadPool::FunctionType> tasks;
        auto& vs = *obj.GetVS();
        ScreenDimension rows_per_thread = (vs.GetUpperRow() - vs.GetLowerRow()) / ThreadsCount;
        for (size_t i = 0; i < ThreadsCount; i++)
        {
            ScreenDimension start = i*rows_per_thread + vs.GetLowerRow();
            ScreenDimension end = (i == ThreadsCount - 1) ? vs.GetUpperRow() : ((i+1)*rows_per_thread + vs.GetLowerRow() - 1);
            tasks.push_back([this, &obj, start, end]()
            {
                rasterizer_.Rasterize(obj, start, end);
            });
        }
        pool_.AddTasks(tasks, true);
    }

    auto const t1 = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> const vs = tv - t0;
    std::chrono::duration<double, std::milli> const fs = t1 - tv;
    std::cout << vs.count() << "; " << fs.count() << "; sum = " << (vs+fs).count() << std::endl;
}

} // namespace plane_render
