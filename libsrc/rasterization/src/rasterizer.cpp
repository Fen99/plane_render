#include "rasterizer.hpp"

#include "rasterization/scene_object.hpp"
#include "rasterization/fragment_shader.hpp"

#include "common/basic_math.hpp"

#include <cmath>
#include <numeric>

namespace plane_render {

Rasterizer::Rasterizer(const RenderingGeometryConstPtr& geom) :
    geom_(geom),
    screen_buffer_(geom_->Width(), geom_->Height())
{
    DCHECK(geom_);
    Clear();
}

void Rasterizer::Rasterize(const SceneObject& obj, size_t start, size_t count)
{
    const VerticesVector& vertices = obj.Vertices();
    const IndicesList& indices = obj.Indices();

    DCHECK(indices.size() % 3 == 0); // Треугольник - 3 точки
    DCHECK(start % 3 == 0);
    for (size_t s = start; s < start+count*3 && s < indices.size(); s += 3)
    {
        const auto& A = vertices[indices[s]];
        const auto& B = vertices[indices[s+1]];
        const auto& C = vertices[indices[s+2]];
        if (A.vertex_coords.z > -GraphicsEps || B.vertex_coords.z > -GraphicsEps || C.vertex_coords.z > -GraphicsEps)
            continue;
        RasterizeTriangle(obj, A, B, C);
    }
}

void Rasterizer::RasterizeTriangle(const SceneObject& obj, const Vertex& A, const Vertex& B, const Vertex& C)
{
    const FragmentShader& fs = *obj.GetFS();

    // alias для координат в px
    const auto& p1_px = A.pixel_pos;
    const auto& p2_px = B.pixel_pos;
    const auto& p3_px = C.pixel_pos;

    // Определяем координаты описывающего прямоугольника
    // px - без Clump, во float. Здесь переводим во вменяемый вид
    PixelPoint mins = { (ScreenDimension) std::max(std::min({p1_px.x, p2_px.x, p3_px.x}), 0.f), 
                        (ScreenDimension) std::max(std::min({p1_px.y, p2_px.y, p3_px.y}), 0.f) };
    PixelPoint maxs = { (ScreenDimension) std::min(std::max({p1_px.x, p2_px.x, p3_px.x}), geom_->Width()-1.f),
                        (ScreenDimension) std::min(std::max({p1_px.y, p2_px.y, p3_px.y}), geom_->Height()-1.f) };

    // Проверяем, валиден ли треугольник - простая проверка
    BaricentricCoords::BCPrecalculated bpc(A, B, C);
    if (!bpc.IsValid()) // Вырожденные треугольники
        return;

    // TODO: переход к следующей строке, если не удалось заблокировать данную
    // Управление растеризацией
    // ScreenDimension rast_low = mins.y; // Растеризация - нижний край
    // ScreenDimension rast_high = -1; // Растеризация - верхний край (первая из пропущенных строк)
    // ScreenDimension notrast_low = -1; // Последний пропущенный ряд

    // ScreenDimension memx_low = -1; // Запоминаем x, где попросили lock на нижнем крае
    // ScreenDimension memx_high = -1; // То же, на первой пропущенной строке
    
    ScreenBuffer::Accessor lines_acc = screen_buffer_.GetAccessor();
    for (ScreenDimension& y_dim = mins.y; y_dim <= maxs.y; y_dim++)
    {
        bool was_pixels = false;
        for (ScreenDimension x_dim = mins.x; x_dim <= maxs.x; x_dim++)
        {
            PixelPoint curr_point = { x_dim, y_dim };
            BaricentricCoords bc(bpc, A, curr_point);
            if (!bc.IsValid())
            {
                if (!was_pixels)
                    continue;
                else
                    break; // Нет смысла дальше идти по этому ряду - треугольника там уже не будет
            }

            was_pixels = true;
            if (lines_acc.LockedRow() == ScreenBuffer::Accessor::INVALID_ROW)
                lines_acc.LockRow(y_dim);
        
            // Проверяем, видна ли точка
            DCHECK((ScreenDimension) lines_acc.LockedRow() == y_dim);
            Vertex avg_vertex = bc.AverageVertices(A, B, C);
            if (avg_vertex.vertex_coords.z < lines_acc.Z(x_dim))
                continue;
            else
                lines_acc.Z(x_dim) = avg_vertex.vertex_coords.z;

            // Отправляем точку во фрагментный шейдер
            lines_acc.Pixel(x_dim) = fs.ProcessFragment(avg_vertex);
        }

        lines_acc.ReleaseRow();
    }
}

} // namespace plane_render
