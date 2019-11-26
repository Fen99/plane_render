#include "scene_object.hpp"

#include "fragment_shader.hpp"
#include "graphics_types.hpp"

#include "common/logger.hpp"

#include <string>
#include <sstream>
#include <fstream>

namespace plane_render {

void SceneObject::VertexShader::Update()
{
    DCHECK(associated_object_->vert_src_coords_.size() == associated_object_->vertices_.size());

    for (size_t i = 0; i < associated_object_->vert_src_coords_.size(); i++)
        associated_object_->geom_->TransformGeometry(associated_object_->vert_src_coords_[i], associated_object_->vertices_[i]);
}

SceneObject::SceneObject(const RenderingGeometryConstPtr& geom, const std::string& obj_filename, float scale,
                         size_t triangles_per_task) :
    geom_(geom),
    triangles_per_task_(triangles_per_task)
{
    LoadMeshFile(obj_filename, scale);
}

SceneObject::SceneObject(const RenderingGeometryConstPtr& geom, const std::vector<Vector3D>& vertices,
                         const std::vector<size_t>& indices, size_t triangles_per_task) : 
    geom_(geom),
    indices_(indices),
    triangles_per_task_(triangles_per_task)
{
    CHECK(indices.size() % 3 == 0);
    for (const auto& v : vertices)
    {
        vert_src_coords_.emplace_back(v.x, v.y, v.z, 1.f);
        vertices_.emplace_back(TextureCoords{0, 0}, Vector3D{0, 1, 0}); // Фиктивная вершина
    }
}

SceneObject::SceneObject(SceneObject&& another) :
    geom_(another.geom_),
    vert_src_coords_(another.vert_src_coords_),
    vertices_(another.vertices_),
    indices_(another.indices_),
    triangles_per_task_(another.triangles_per_task_),
    vs_(another.vs_),
    fs_(another.fs_)
{
    // Вершинный шейдер перенастраиваем на нас
    vs_->associated_object_ = this;

    another.vs_ = nullptr;
    another.fs_ = nullptr;
}

SceneObject::~SceneObject()
{
    delete vs_;
    delete fs_;
}

void SceneObject::LoadMeshFile(const std::string& obj_filename, float scale)
{
    std::ifstream in(obj_filename);
    if(!in.is_open())
        throw std::invalid_argument("can not find file " + std::string(obj_filename));

    std::string line;
    std::vector<Vector3D> pos;
    std::vector<TextureCoords> tex;
    std::vector<Vector3D> norm;
    while(std::getline(in, line))
    {
        std::istringstream iss(line);
        std::string word;
        iss >> word;
        if(word == "v")
        {
            Vector3D v;
            if(!(iss >> v.x >> v.y >> v.z))
                throw std::runtime_error("could not parse line: " + line);
            v.x *= scale;
            v.y *= scale;
            v.z *= scale;

            pos.push_back(v);
        }
        else if(word == "vt")
        {
            TextureCoords v;
            if(!(iss >> v.x >> v.y))
                throw std::runtime_error("could not parse line: " + line);
            if (v.x > 1 || v.y > 1 || v.x < 0 || v.y < 0)
                LOG(ERROR) << "wrong texture coords: "+line;
            v.x = Clump(v.x, 0.001f, 0.999f);
            v.y = Clump(v.y, 0.001f, 0.999f);
            tex.push_back(v);
        }
        else if(word == "vn")
        {
            Vector3D v;
            if(!(iss >> v.x >> v.y >> v.z))
                throw std::runtime_error("could not parse line: " + line);
            norm.push_back(v.Normalized());
        }
        else if(word == "f")
        {
            size_t idx[3] = {};
            char trash;
            auto const vsize = vert_src_coords_.size();
            while (iss >> idx[0] >> trash >> idx[1] >> trash >> idx[2])
            {
                if (static_cast<int>(idx[2]) - 1 != -1) // FIXME: Хак, чтобы использовать этот парсер для "неполных" obj
                {
                    vert_src_coords_.emplace_back(pos[idx[0] - 1], 1.f);
                    vertices_.emplace_back(tex[idx[1] - 1], norm[idx[2] - 1]);
                }
                else
                {
                    vert_src_coords_.emplace_back(pos[idx[0] - 1], 1.f);
                    vertices_.emplace_back(tex[idx[1] - 1], Vector3D{0, 1, 0}); // Фиктивная нормаль
                }
            }

            auto const vcount = vert_src_coords_.size() - vsize;
            if (vcount == 3)
            {
                indices_.push_back(vsize);
                indices_.push_back(vsize + 1);
                indices_.push_back(vsize + 2);
            }
            else if (vcount == 4)
            {
                indices_.push_back(vsize);
                indices_.push_back(vsize + 1);
                indices_.push_back(vsize + 2);

                indices_.push_back(vsize + 2);
                indices_.push_back(vsize);
                indices_.push_back(vsize + 3);
            }
            else
                throw std::runtime_error("Faces not supported!");
        }
    }

    DCHECK(vert_src_coords_.size() == vertices_.size());
    DCHECK(indices_.size() % 3 == 0);
}

void SceneObject::Update()
{
    vs_->Update();
}

} // namespace plane_render