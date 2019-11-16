#include "rasterizer.hpp"

#include "rasterization/scene_object.hpp"
#include "rasterization/vertex_shader.hpp"
#include "rasterization/fragment_shader.hpp"

#include "common/math_simple.hpp"

#include <Eigen/Dense>
#include <cmath>
#include <numeric>

namespace plane_render {

Rasterizer::Rasterizer(const RenderingGeometryConstPtr& geom) :
    geom_(geom)
{
    DCHECK(geom_);

    pixels_ = new Color[geom_->PixelsCount()];
    z_buffer_ = new float[geom_->PixelsCount()];
    CHECK_ALWAYS_COMMENT(pixels_ && z_buffer_, "memory allocation error");

    Clear();
}

Rasterizer::~Rasterizer()
{
    delete[] pixels_;
    delete[] z_buffer_;
}

void Rasterizer::Clear()
{
    for (size_t i = 0; i < geom_->PixelsCount(); i++)
        z_buffer_[i] = -std::numeric_limits<float>::max();
    memset(pixels_, 0, geom_->PixelsCount() *sizeof(Color));
}

void Rasterizer::Rasterize(const SceneObject& object, ScreenDimension min_line, ScreenDimension max_line)
{
    DCHECK(min_line >= 0 && max_line >= 0 && min_line < geom_->Height() && max_line < geom_->Height() &&
          min_line <= max_line);

    const auto& vs = *object.GetVS();
    if (vs.GetLowerRow() > max_line || vs.GetUpperRow() < min_line)
        return;

    const auto& indices = object.Indices();
    DCHECK(indices.size() % 6 == 0); // Треугольник - 3 линии

    for (size_t i = 0; i < indices.size() / 6; i++)
    {
        // Проверяем, что точки действительно образуют треугольник
        DCHECK(indices[i*6+1] == indices[i*6+2] &&
              indices[i*6+3] == indices[i*6+4] &&
              indices[i*6+5] == indices[i*6]);
        RasterizeTriangle(object, min_line, max_line, { indices[i*6], indices[i*6+1], indices[i*6+3] });
    }
}

void Rasterizer::RasterizeTriangle(const SceneObject& obj, ScreenDimension min_line, ScreenDimension max_line,
                                   const TriangleIndices& indices)
{
    const VertexShader& vs = *obj.GetVS();
    const FragmentShader& fs = *obj.GetFS();

    // Геометрия
    const Vector3D& v1_3d = *vs.GetPoint(indices.v1);
    const Vector3D& v2_3d = *vs.GetPoint(indices.v2);
    const Vector3D& v3_3d = *vs.GetPoint(indices.v3);

    if (v1_3d.z < -1 || v2_3d.z < -1 || v3_3d.z < -1)
        return;

    const PixelPoint& p1_px = *vs.GetAsPixelPoint(indices.v1);
    const PixelPoint& p2_px = *vs.GetAsPixelPoint(indices.v2);
    const PixelPoint& p3_px = *vs.GetAsPixelPoint(indices.v3);

    // Свойства
    const Vertex& vert1 = obj.GetVertex(indices.v1);
    const Vertex& vert2 = obj.GetVertex(indices.v2);
    const Vertex& vert3 = obj.GetVertex(indices.v3);

    // Определяем координаты описывающего прямоугольника
    // Px - уже с Clump
    PixelPoint mins = { std::max(std::min({p1_px.x, p2_px.x, p3_px.x}), 0), 
                        std::max(std::min({p1_px.y, p2_px.y, p3_px.y}), 0) };
    PixelPoint maxs = { std::min(std::max({p1_px.x, p2_px.x, p3_px.x}), geom_->Width()-1),
                        std::min(std::max({p1_px.y, p2_px.y, p3_px.y}), geom_->Height()-1) };

    // Проверяем, может ли данный поток нарисовать хоть что-то в этом треугольнике
    if (mins.y > max_line || maxs.y < min_line)
        return;
    // Проверяем, валиден ли треугольник
    if (mins.x == maxs.x || mins.y == maxs.y)
        return;

    for (ScreenDimension y_dim = std::max(mins.y, min_line); y_dim <= std::min(maxs.y, max_line); y_dim++)
     for (ScreenDimension x_dim = mins.x; x_dim <= maxs.x; x_dim++)
     {
         PixelPoint curr_point = { x_dim, y_dim };
         size_t idx = CalcIndex(curr_point);
         
         // Точно корректный
         //if (!ValidateIndex(curr_point))
         //    continue;

         BaricentricCoords screen_bc = ToScreenBaricentric(curr_point, p1_px, p2_px, p3_px);
         //if (!screen_bc.IsValid())
         //    continue;
         BaricentricCoords world_bc = screen_bc;
         //BaricentricCoords world_bc = ToWorldBaricentric(screen_bc, vert1.vreg.position.z, vert2.vreg.position.z, 
         //                                                vert3.vreg.position.z);
         if (!world_bc.IsValid())
            continue;

         // Проверяем, видна ли поверхность
         Vertex avg_vertex = FragmentShader::AgregateVertices(world_bc, vert1, vert2, vert3);
         float dzeta_avg = world_bc.a*v1_3d.z + world_bc.b*v2_3d.z + world_bc.c*v3_3d.z;
         //float dzeta_avg = screen_bc.a*v1_3d.z + screen_bc.b*v2_3d.z + screen_bc.c*v3_3d.z;
         if (dzeta_avg <= z_buffer_[idx])
             continue;
         else
             z_buffer_[idx] = dzeta_avg;

         // Отправляем точку во фрагментный шейдер
         /*if (avg_vertex.vreg.properties.texture_coords.x > 1 || avg_vertex.vreg.properties.texture_coords.y > 1)
         {
             std::cout << ">>> " << vert1.vreg.properties.texture_coords.x << " " << vert1.vreg.properties.texture_coords.y << std::endl;
             std::cout << ">>> " << vert2.vreg.properties.texture_coords.x << " " << vert2.vreg.properties.texture_coords.y << std::endl;
             std::cout << ">>> " << vert3.vreg.properties.texture_coords.x << " " << vert3.vreg.properties.texture_coords.y << std::endl;
             std::cout << "wb>>> " << world_bc.a << " " << world_bc.b << " " << world_bc.c << std::endl;
             std::cout << "sc>>> " << screen_bc.a << " " << screen_bc.b << " " << screen_bc.c << std::endl;
             std::cout << "z>>> " << vert1.vreg.position.z << " " << vert2.vreg.position.z << " " << vert3.vreg.position.z << std::endl;
             std::cout << ">><<" << std::endl; 
         }*/
         pixels_[idx] = fs.ProcessFragment(avg_vertex);
         //pixels_[CalcIndex(curr_point)] = tri_color;
     }
}

} // namespace plane_render
