#include "scene_object.hpp"

#include "vertex_shader.hpp"
#include "fragment_shader.hpp"
#include "graphics_types.hpp"

#include "common/logger.hpp"

#include <string>
#include <sstream>
#include <fstream>

namespace plane_render {

SceneObject::SceneObject(const RenderingGeometryConstPtr& geom, const std::string& obj_filename, float scale) :
    geom_(geom)
{
    LoadMeshFile(obj_filename, scale);
}

SceneObject::SceneObject(SceneObject&& another) :
    geom_(another.geom_),
    vertices_(another.vertices_),
    indices_(another.indices_),
    vs_(another.vs_),
    fs_(another.fs_)
{
    another.vs_ = nullptr;
    another.fs_ = nullptr;
}

SceneObject::~SceneObject()
{
    AlignmentAllocator<void, 16>().deallocate(vs_, 0); // Здесь тип аллокатора и размер не важны

    delete fs_;
}

void SceneObject::LoadMeshFile(const std::string& obj_filename, float scale)
{
    std::ifstream in(obj_filename);
    if(!in.is_open())
        throw std::invalid_argument("can not find file " + std::string(obj_filename));

    std::string line;
    std::vector<WorldPoint> pos;
    std::vector<TextureCoords> tex;
    std::vector<Vector3D> norm;
    while(std::getline(in, line))
    {
        std::istringstream iss(line);
        std::string word;
        iss >> word;
        if(word == "v")
        {
            WorldPoint v;
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
            if (v.x > 1 || v.y > 1)
                throw std::runtime_error("wrong texture coords: "+line);
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
            auto const vsize = vertices_.size();
            while (iss >> idx[0] >> trash >> idx[1] >> trash >> idx[2])
            {
                if (static_cast<int>(idx[2]) - 1 != -1) // FIXME: Хак, чтобы использовать этот парсер для "неполных" obj
                    vertices_.emplace_back(pos[idx[0] - 1], tex[idx[1] - 1], norm[idx[2] - 1]);
                else
                    vertices_.emplace_back(pos[idx[0] - 1], tex[idx[1] - 1], Vector3D{0, 1, 0}); // Фиктивная нормаль
            }

            auto const vcount = vertices_.size() - vsize;
            if (vcount == 3)
            {
                indices_.push_back(vsize);     indices_.push_back(vsize + 1);
                indices_.push_back(vsize + 1); indices_.push_back(vsize + 2);
                indices_.push_back(vsize + 2); indices_.push_back(vsize);
            }
            else if (vcount == 4)
            {
                indices_.push_back(vsize);     indices_.push_back(vsize + 1);
                indices_.push_back(vsize + 1); indices_.push_back(vsize + 2);
                indices_.push_back(vsize + 2); indices_.push_back(vsize);


                indices_.push_back(vsize + 2); indices_.push_back(vsize);
                indices_.push_back(vsize); indices_.push_back(vsize + 3);
                indices_.push_back(vsize + 3); indices_.push_back(vsize + 2);
            }
            else
                throw std::runtime_error("Faces not supported!");
        }
    }
}

void SceneObject::Update()
{
    vs_->Update();
}

} // namespace plane_render