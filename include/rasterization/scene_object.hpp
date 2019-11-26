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
    class VertexShader
    {
    // Создавать объекты вершинного шейдера и пользоваться ими должен только SceneObject
    // Этой же логики должны придерживаться наследники
    protected:
        VertexShader(SceneObject* object) : associated_object_(object) {}
        virtual ~VertexShader() {}

        virtual void Update(); // Берет информацию из RenderingInfo о движении камеры

    protected:
        SceneObject* associated_object_;
        friend class SceneObject;
    };

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
        vs_ = new VS(this);
        fs_ = new FS(geom_);
    }

    // Запускает вершинный шейдер для перерасчета (при обновлении позиции камеры)
    void Update();

    const IndicesList&    Indices()  const { return indices_;  }
    const VerticesVector& Vertices() const { return vertices_; }

    const VertexShader*   GetVS() const { return vs_; }
    const FragmentShader* GetFS() const { return fs_; }
    FragmentShader* GetFS() { return fs_; } // Для работы с текстурами и т.п.

private:
    void LoadMeshFile(const std::string& obj_filename, float scale);

private:
    RenderingGeometryConstPtr geom_;

    Vec4DynamicArray vert_src_coords_; // Координаты из меша
    VerticesVector vertices_; // Свойства вершин для растеризатора (меняются на каждой итерации)
    IndicesList indices_;

    VertexShader* vs_   = nullptr;
    FragmentShader* fs_ = nullptr;

    // Вершинный шейдер может менять свойства вершин и иметь доступ к исходным координатам
    friend class VertexShader;
};

} // namespace plane_render