#include "pipeline.hpp"

#include "fragment_shader.hpp"

#include <chrono>

namespace plane_render {

RasterizationPipeline::RasterizationPipeline(const RenderingGeometryPtr& geom,
                                             std::vector<SceneObject>&& objects, const std::string& perf_filename) :
    geom_(geom),
    objects_(std::move(objects)),
    pool_(ThreadsCount),
    rasterizer_(geom_),
    perf_output_(perf_filename, std::ios_base::out)
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
    perf_output_ << (vs+fs).count() << "\t" << vs.count() << "\t" << fs.count() << std::endl;
}

} // namespace plane_render
