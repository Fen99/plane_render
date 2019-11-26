#pragma once

#include "graphics_types.hpp"
#include "common/logger.hpp"

#include <memory>

namespace plane_render {

// Общая информация о сцене: позиция камеры, света + методы преобразования геометрии в экранную
struct alignas(16) RenderingGeometry
{
public:
    RenderingGeometry(ScreenDimension w, ScreenDimension h, float n_p, float f_p, float fov);
    void* operator new (size_t bytes) { return AlignmentAllocator<RenderingGeometry, 16>().allocate(bytes); }
    void operator delete(void* ptr) { AlignmentAllocator<void, 16>().deallocate(ptr, 0); }

    void Move(const Vector3D& mov);
    void Rotate(const RotationAngles& rot);
    
    void LookAt(const Vector3D& pos, const RotationAngles& angles);

    // Преобразование геометрии для вершинных шейдеров
    // src_vec4 - (x, y, z, 1.0) - исходный 4d-вектор координат
    // out_vp - Vertex, соответствующая вершине. В ней заполняются vertex_coords, pixel_pos
    // (!) При подаче в растеризатор нужно проверить, что z_вершины <= -GraphicsEps - нельзя рисовать точки с z >= 0
    void TransformGeometry(const Vector4D& src_vec4, Vertex& out_v) const;

    ScreenDimension Width()  const { return screen_width_;  }
    ScreenDimension Height() const { return screen_height_; }

    const FastVector3D& CameraPosSrc() const { return camera_pos_src_; }
    const FastVector3D& LightPosSrc()  const { return light_pos_src_;  }
    const FastVector3D& LightPos()     const { return light_pos_;      }

    void SetLightSrcPos(const Vector3D& pos);

private:
    void SetToPixelsCoeffitients();
    void UpdateTransform(); // Пересчитывает матрицы и свет

private:
    static const FastVector3D UP;

private:
    // Преобразования геометрии
    Matrix4 perspective_; // Считаем 1 раз
    Matrix4 rotation_;
    Matrix4 result_space_; // rotation*move

    // Вектора для преобразования экранных координат в пиксельные
    __m128 topixels_mul_;
    __m128 topixels_add_;

    FastVector3D light_pos_src_ = {0, 0, 0}; // Исходная
    FastVector3D light_pos_ = {0, 0, 0}; // Повернутая и смещенная
    FastVector3D camera_pos_src_ = {0, 0, 0}; // Исходная. Повернутая и смещенная смысла не имеет - она (0; 0; 0)

    // Геометрия экрана
    ScreenDimension screen_width_ = 0;
    ScreenDimension screen_height_ = 0;

    float n_ = 0; // Ближний план 
    float f_ = 0; // Дальный план
    float fov_ = 1.f;
};

typedef std::shared_ptr<RenderingGeometry> RenderingGeometryPtr;
typedef std::shared_ptr<const RenderingGeometry> RenderingGeometryConstPtr;

} // namespace plane_render