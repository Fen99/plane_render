#pragma once

#include "common/aligned_allocator.hpp"
#include "common/basic_math.hpp"
#include "common/logger.hpp"
#include "common/common_graphics.hpp"

#include <functional>
#include <utility>
#include <vector>
#include <memory>
#include <cmath>

namespace plane_render {

// Для проверок float-значений на предмет нуля
constexpr float GraphicsEps = (float) 1e-10;

// Objects
using TextureCoords = Point2D<float>;
struct alignas(16) Vertex
{
public:
    // 4 float. Должны хранить координату (ДО перспективы) для фрагментного шейдера
    FastVector3D vertex_coords;

    // 4 float
    FastVector3D normal;

    // 4 float
    union
    {
        struct
        {
            TextureCoords texture_coords;
            PixelPointF pixel_pos; // 2 float: для выравнивания + заодно храним данные. Без clump!
        };
        __m128 vp3;
    };

    // Первичное создание при загрузке из файла
    Vertex(const TextureCoords& tex, const Vector3D& norm) :
        normal(norm), texture_coords{tex}
    {
        DCHECK_ALIGNMENT_16;
    }
    Vertex(__m128 vp1, __m128 vp2, __m128 vp3) :
        vertex_coords(vp1), normal(vp2), vp3(vp3)
    {
        DCHECK_ALIGNMENT_16;
    }

    // Достает 2 младших float из pixel_pos и устанавливает их в соответствующее поле
    inline void SetPixelPos(__m128 pixel_pos)
    {
        // a = vp2 - сохраняет текстурные координаты, b = pixel_pos - забирает младшие байты
        vp3 = _mm_movelh_ps(vp3, pixel_pos);
    }

    inline Vertex operator*(float val) const
    {
        return { vertex_coords*val, normal*val, _mm_mul_ps(vp3, _mm_set1_ps(val)) };
    }

    inline Vertex operator+(const Vertex& v_second) const
    {
        return { vertex_coords + v_second.vertex_coords, normal + v_second.normal, _mm_add_ps(vp3, v_second.vp3) };
    }
};
typedef VectorAlignment16<Vertex> VerticesVector;

// Triangles
typedef std::vector<size_t> IndicesList;

// Барицентрические координаты
// Приватно наследуемся, чтобы скрыть +, -, *
struct alignas(16) BaricentricCoords : private Vector4D
{
public:
    // Информация для расчета БЦ-координат, которая зависит только от вершин треугольника
    struct alignas(16) BCPrecalculated
    {
    private:
        __m128 z_inv_; // 1/z1, 1/z2, 1/z3, 1.0 - для скалярного произведения с ним в пересчете в мировые
        Point2D<float> BA_d_; // _d - уже с делением на знаменатель: 3й элемент векторного произведения
        Point2D<float> CA_d_;
        float denom_;

    public:
        // Вызывающий обязан: 1) проверить denom != 0.f: не является ли треугольник вырожденным - IsValid()
        //                    2) обеспечить abs(z_coord) <= -GraphicsEps
        inline BCPrecalculated(const Vertex& A, const Vertex& B, const Vertex& C) :
            BA_d_{B.pixel_pos.x - A.pixel_pos.x,
                  B.pixel_pos.y - A.pixel_pos.y},
            CA_d_{C.pixel_pos.x - A.pixel_pos.x,
                  C.pixel_pos.y - A.pixel_pos.y}
        {
            DCHECK_ALIGNMENT_16;
            // Нельзя использовать точки с положительными и нулевыми z
            DCHECK(A.vertex_coords.z <= -GraphicsEps && B.vertex_coords.z <= -GraphicsEps && C.vertex_coords.z <= -GraphicsEps);

            denom_ = BA_d_.x*CA_d_.y - CA_d_.x*BA_d_.y;
            // Замедляет, но не имеет эффекта
            //if (std::abs(denom_) < GraphicsEps) // Вырожденный треугольник
            //{
            //    denom_ = 0.f;
            //    return;
            //}

            BA_d_.x /= denom_; BA_d_.y /= denom_;
            CA_d_.x /= denom_; CA_d_.y /= denom_;

            z_inv_ = _mm_set_ps(0.f, 1/C.vertex_coords.z, 1/B.vertex_coords.z, 1/A.vertex_coords.z);
        }
        inline bool IsValid() const { return denom_ != 0.f; }

        friend struct BaricentricCoords;
    };

private:
    bool is_valid_ = true;

public:
    /*  Point2D<float> BA{(float) B.x - A.x, (float) B.y - A.y};
        Point2D<float> CA{(float) C.x - A.x, (float) C.y - A.y};
        Point2D<float> AP{(float) A.x - p.x, (float) A.y - p.y};
        float denom = BA.x*CA.y - CA.x*BA.y;

        float comp_b = (CA.x*AP.y - AP.x*CA.y) / denom;
        float comp_c = (AP.x*BA.y - BA.x*AP.y) / denom;
        BaricentricCoords res_new = { 1.f - comp_b - comp_c, comp_b, comp_c }; */

    // Нельзя использовать с невалидным BCPrecalculated (где denom = 0 - вырожденный треугольник)
    // Вершина A - чтобы посчитать AP
    // После вызова проверять IsValid()!
    inline BaricentricCoords(const BCPrecalculated& bcp, const Vertex& A, const PixelPoint& p)
    {
        DCHECK_ALIGNMENT_16;
        DCHECK(std::abs(bcp.denom_) >= GraphicsEps);

        Point2D<float> AP{ A.pixel_pos.x - p.x, A.pixel_pos.y - p.y};
        float comp_b = bcp.CA_d_.x*AP.y - AP.x*bcp.CA_d_.y;
        float comp_c = AP.x*bcp.BA_d_.y - bcp.BA_d_.x*AP.y;
        v4 = _mm_set_ps(0.f, comp_c, comp_b, 1.f - comp_b - comp_c);

        // Проверяем, все ли значения >= 0
        __m128 comp_raw = _mm_cmplt_ps(v4, _mm_set1_ps((float) -1e-4));
        int comp = _mm_movemask_ps(comp_raw);
        if (comp)
        {
            is_valid_ = false;
            return;
        }

        __m128 summ = _mm_dp_ps(v4, bcp.z_inv_, 0x7F); // Перемножаем без fourth, кладем во все
        v4 = _mm_mul_ps(v4, bcp.z_inv_); // Барицентрики до деления на суммы
        v4 = _mm_div_ps(v4, summ); // Получаем мировые БЦ
    }

    inline bool IsValid() const { return is_valid_; }

    // Нельзя использовать если !IsValid
    inline Vertex AverageVertices(const Vertex& A, const Vertex& B, const Vertex& C) const
    {
        DCHECK(IsValid());
        return A*x + B*y + C*z;
    }
};

} // namespace plane_render
