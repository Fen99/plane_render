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
    CHECK(geom_);

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

bool Rasterizer::ValidateIndex(PixelPoint point) const
{
    return point.x < geom_->Width() && point.y < geom_->Height() && point.x >= 0 && point.y >= 0;
}

size_t Rasterizer::CalcIndex(PixelPoint point) const
{
    CHECK(ValidateIndex(point));
    return point.y*geom_->Width() + point.x;
}

Color& Rasterizer::At(const PixelPoint& point)
{
    return pixels_[CalcIndex(point)];
}

void Rasterizer::Rasterize(const SceneObject& object, ScreenDimension min_line, ScreenDimension max_line)
{
    CHECK(min_line >= 0 && max_line >= 0 && min_line < geom_->Height() && max_line < geom_->Height() &&
          min_line <= max_line);

    const auto& vs = *object.GetVS();
    if (vs.GetLowerRow() > max_line || vs.GetUpperRow() < min_line)
        return;

    const auto& indices = object.Indices();
    CHECK(indices.size() % 6 == 0); // Треугольник - 3 линии

    for (size_t i = 0; i < indices.size() / 6; i++)
    {
        // Проверяем, что точки действительно образуют треугольник
        CHECK(indices[i*6+1] == indices[i*6+2] &&
                indices[i*6+3] == indices[i*6+4] &&
                indices[i*6+5] == indices[i*6]);
        RasterizeTriangle(object, min_line, max_line, { indices[i*6], indices[i*6+1], indices[i*6+3] });
    }
}

BaricentricCoords Rasterizer::ToScreenBaricentric(const PixelPoint& p, const PixelPoint& A, const PixelPoint& B, const PixelPoint& C)
{
    Vector3D x_vec = { (float) B.x - A.x, (float) C.x - A.x, (float) A.x - p.x };
    Vector3D y_vec = { (float) B.y - A.y, (float) C.y - A.y, (float) A.y - p.y };
    Vector3D cross = x_vec.Cross(y_vec);
    return { 1.f - (cross.x+cross.y)/cross.z, cross.x/cross.z, cross.y/cross.z };
}

BaricentricCoords Rasterizer::ToWorldBaricentric(const BaricentricCoords& screen_bc, float z1, float z2, float z3)
{
    float alphap = screen_bc.a / z1;
    float betap  = screen_bc.b / z2;
    float gammap = screen_bc.c / z3;
    float summ = alphap + betap + gammap;

    return { alphap/summ, betap/summ, gammap/summ };
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
         if (!screen_bc.IsValid())
             continue;
         BaricentricCoords world_bc = ToWorldBaricentric(screen_bc, vert1.vreg.position.z, vert2.vreg.position.z, 
                                                         vert3.vreg.position.z);

         // Проверяем, видна ли поверхность
         Vertex avg_vertex = FragmentShader::AgregateVertices(world_bc, vert1, vert2, vert3);
         float dzeta_avg = world_bc.a*v1_3d.z + world_bc.b*v2_3d.z + world_bc.c*v3_3d.z;
         if (dzeta_avg <= z_buffer_[idx])
             continue;
         else
             z_buffer_[idx] = dzeta_avg;

         // Отправляем точку во фрагментный шейдер
         pixels_[idx] = fs.ProcessFragment(avg_vertex);
         //pixels_[CalcIndex(curr_point)] = tri_color;
     }
}

} // namespace plane_render
