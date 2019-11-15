#pragma once

#include "rasterization/rendering_geometry.hpp"
#include "rasterization/graphics_types.hpp"

#include "common/aligned_allocator.hpp"

#include <string>
#include <sstream>
#include <fstream>

namespace plane_render {

class VertexShader;
class FragmentShader;

class SceneObject
{
public:
    typedef std::vector<size_t> IndicesList;

public:
    SceneObject(const RenderingGeometryConstPtr& geom, const std::string& obj_filename, float scale = 1.0);
    SceneObject(SceneObject&& another);
    ~SceneObject();

    SceneObject& operator=(const SceneObject&) = delete;
    SceneObject(const SceneObject&) = delete;

    // После создания устанавливает шейдеры - должна быть вызвана!
    template<typename VS, typename FS>
    void SetShaders()
    {
        void* vs_memory = AlignmentAllocator<VS, 16>().allocate(1); // Аллоцируем 1 объект без конструирования
        vs_ = new(vs_memory) VS(vertices_, geom_);
        fs_ = new FS(geom_);
    }

    // Запускает вершинный шейдер для перерасчета (при обновлении позиции камеры)
    void Update();

    const IndicesList& Indices() const { return indices_; }
    const Vertex& GetVertex(size_t index) const { return vertices_[index]; }

    const VertexShader*   GetVS() const { return vs_; }
    const FragmentShader* GetFS() const { return fs_; }
    FragmentShader* GetFS() { return fs_; } // Для работы с текстурами и т.п.

private:
    void LoadMeshFile(const std::string& obj_filename, float scale);

private:
    RenderingGeometryConstPtr geom_;

    VeticesVector vertices_;
    IndicesList indices_;

    VertexShader* vs_   = nullptr;
    FragmentShader* fs_ = nullptr;
};

} // namespace plane_render