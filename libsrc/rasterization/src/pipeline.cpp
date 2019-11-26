#include "pipeline.hpp"

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
    std::chrono::duration<double, std::milli> const vs = tv - t0;
    DLOG(INFO) << "Interations: " << iterations_ << "; vs_debug: = " << vs.count() << " ms" << std::endl;

    for (auto& obj : objects_)
    {
        size_t ind_count = obj.Indices().size();
        DCHECK(ind_count % 3 == 0);
        size_t triang_count = ind_count / 3;
        size_t triangles_per_thread = triang_count / ThreadsCount + 1; // +1, чтобы точно все нарисовались

        for (size_t st = 0; st < triangles_per_thread; st += obj.TrianglesPerTask())
         for (size_t th = 0; th < ThreadsCount; th++)
         {
             size_t start = th*triangles_per_thread + st; // Номер первого треугольника
             size_t count = obj.TrianglesPerTask(); // Сколько треугольников
             if (start + count > (th+1)*triangles_per_thread) // Залезли уже на чужую территорию
                count = (th+1)*triangles_per_thread - start;

             pool_.AddTask([this, &obj, start, count]()
                           {
                                rasterizer_.Rasterize(obj, start*3, count);
                           }, false);
         }

        pool_.Join();
    }
    auto const t1 = std::chrono::system_clock::now();
    std::chrono::duration<double, std::milli> const fs = t1 - tv;
    sum_time_.first += vs.count();
    sum_time_.second += fs.count();
    iterations_++;

    std::cout << iterations_ << ": " << vs.count() << "; " << fs.count() << "; sum = " << (vs+fs).count() << std::endl;
    if (iterations_ > 0 && iterations_ % 100 == 0)
    {
        std::cout << "Avg. after " << iterations_ << " iterations: " << sum_time_.first / iterations_ << "; " <<
                     sum_time_.second / iterations_ << "; sum_avg = " << (sum_time_.first + sum_time_.second) / iterations_ << std::endl;
    }
}

} // namespace plane_render
