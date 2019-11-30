#pragma once

#include "graphics_types.hpp"
#include "common/logger.hpp"

#include <memory>

namespace plane_render {

// Общая информация о сцене: позиция камеры, света + методы преобразования геометрии в экранную
struct alignas(16) RenderingGeometry
{
public:
    RenderingGeometry(ScreenDimension w, ScreenDimension h, float n_p, float f_p, float fov,
                      const Vector3D& up = {0.f, 1.f, 0.f});
    void* operator new (size_t bytes) { return AlignmentAllocator<RenderingGeometry, 16>().allocate(bytes); }
    void operator delete(void* ptr) { AlignmentAllocator<void, 16>().deallocate(ptr, 0); }

    // pos - позиция камеры
    // at - в какую точку мы смотрим (какая у нас в центре экрана)
    // pos - at = dir - вектор направления
    void MoveCam(const Vector3D& mov); // pos = pos + mov
    void MoveAt(const Vector3D& at_shift); // at = at + at_shift
    void LookAt(const Vector3D& pos, const Vector3D& at); // Полностью перезаписывает pos и at

    // Вспомогательная функция для преобразований геометрии
    // Переводит экранные координаты (ksi, eta, dzeta) в пиксели и устанавливает их в out_v
    inline void SetPixelPos(const FastVector3D& coords_screen, Vertex& out_v) const
    {
        /*   
        return { Clump((ScreenDimension) std::round(-1.0/2 + screen_width_ / 2.0*(ksi + 1)), 0, screen_width_-1),
                 Clump((ScreenDimension) std::round(-1.0/2 + screen_height_ / 2.0*(eta + 1)), 0, screen_height_-1) }; */

        __m128 pixel_pos = _mm_add_ps(_mm_mul_ps(coords_screen, topixels_mul_), topixels_add_);
        pixel_pos = _mm_round_ps(pixel_pos, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
        out_v.SetPixelPos(pixel_pos);
    }

    // Преобразование геометрии для вершинных шейдеров
    // src_vec4 - (x, y, z, 1.0) - исходный 4d-вектор координат
    // out_vp - Vertex, соответствующая вершине. В ней заполняются vertex_coords, pixel_pos
    // (!) При подаче в растеризатор нужно проверить, что z_вершины <= -GraphicsEps - нельзя рисовать точки с z >= 0
    void TransformGeometry(const Vector4D& src_vec4, Vertex& out_v) const;

    ScreenDimension Width()  const { return screen_width_;  }
    ScreenDimension Height() const { return screen_height_; }

    const FastVector3D& CameraPosSrc() const { return camera_pos_src_; } // После преобразования - в (0, 0, 0)
    const FastVector3D& LightPosSrc()  const { return light_pos_src_;  } // Свет до преобразования
    const FastVector3D& LightPos()     const { return light_pos_;      } // Свет после преобразования

    // Для работы с raytracing
    float GetRatio() const { return ratio_; }
    float GetFov()   const { return fov_; }
    Vector3D GetUp() const { return up_.ToVector3D(); }
    Vector3D GetAt() const { return at_.ToVector3D(); }

    void SetLightSrcPos(const Vector3D& pos);

private:
    void SetToPixelsCoeffitients();
    void UpdateTransform(); // Пересчитывает матрицы и свет

private:
    // Положение камеры и вида
    FastVector3D up_ = {0, 1, 0};
    FastVector3D at_ = {0, 0, 1}; // Чтобы изначально смотрели вдоль z
    FastVector3D camera_pos_src_ = {0, 0, 0};

    Matrix4 perspective_; // Считаем 1 раз
    Matrix4 result_space_; // rotation*move

    // Вектора для преобразования экранных координат в пиксельные
    __m128 topixels_mul_;
    __m128 topixels_add_;

    FastVector3D light_pos_src_ = {0, 0, 0}; // Исходная
    FastVector3D light_pos_ = {0, 0, 0}; // Повернутая и смещенная

    // Геометрия экрана
    ScreenDimension screen_width_ = 0;
    ScreenDimension screen_height_ = 0;
    float ratio_ = 0.f;

    float n_ = 0; // Ближний план
    float f_ = 0; // Дальный план
    float fov_ = 1.f;
};

typedef std::shared_ptr<RenderingGeometry> RenderingGeometryPtr;
typedef std::shared_ptr<const RenderingGeometry> RenderingGeometryConstPtr;

} // namespace plane_render