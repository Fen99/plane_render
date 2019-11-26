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
    // Этой же логики должны придерживаться наследники (закрывать конструктор, объявлять другом SceneObject)
    // Для конструктора лучше использовать using VertexShader::VertexShader;
    protected:
        SceneObject* associated_object_ = nullptr;
        friend class SceneObject;

    protected:
        VertexShader(SceneObject* object) : associated_object_(object) {}
        virtual ~VertexShader() {}

        // Функция должна перевести массив исходных координат GetAssociatedSrcCoords()
        // в трансформированную геометрию GetAssociatedVertices()
        // (!) Перевод в px - тоже на вершинном шейдере!
        virtual void Update(); // Берет информацию из RenderingInfo о движении камеры
    
    // Наследникам нужны данные для работы
    protected:
        const RenderingGeometry& GetGeom() const { return *associated_object_->geom_.get(); }
        VerticesVector& GetAssociatedVertices() { return associated_object_->vertices_; }
        const Vec4DynamicArray& GetAssociatedSrcCoords() const { return associated_object_->vert_src_coords_; }
    };

public:
    SceneObject(const RenderingGeometryConstPtr& geom, const std::string& obj_filename, float scale = 1.0,
                size_t triangles_per_task = 1000);
    
    // Создание объекта без нормалей и текстурных координат. Это должен учитывать фрагментный шейдер!
    SceneObject(const RenderingGeometryConstPtr& geom, const std::vector<Vector3D>& vertices,
                const std::vector<size_t>& indices, size_t triangles_per_task = 1000);

    SceneObject(SceneObject&& another);
    ~SceneObject();

    SceneObject& operator=(SceneObject&&) = delete;
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

    size_t TrianglesPerTask() const { return triangles_per_task_; } // Индивидуально для каждого объекта
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

    const size_t triangles_per_task_ = 0;

    VertexShader* vs_   = nullptr;
    FragmentShader* fs_ = nullptr;

    // Вершинный шейдер может менять свойства вершин и иметь доступ к исходным координатам
    friend class VertexShader;
};

} // namespace plane_render