#pragma once

#include "graphics_types.hpp"
#include "common/logger.hpp"

#include <Eigen/Dense>
#include <memory>

namespace plane_render {

// Общая информация о сцене: позиция камеры, света
// + методы преобразования геометрии в экранную
struct RenderingGeometry
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    typedef Eigen::Matrix<float, 4, Eigen::Dynamic> SrcGeometryMatrix; // 4D- вектора (x, y, z, 1)
    
    typedef Eigen::Matrix<float, 4, 4> TransformMatrix;
    typedef Eigen::Matrix<float, 3, Eigen::Dynamic> TransformedGeometryMatrix; // (ksi, eta, dzeta)
    typedef Eigen::Matrix<ScreenDimension, 2, Eigen::Dynamic> PixelsMatrix;

public:
    RenderingGeometry(ScreenDimension w, ScreenDimension h, float n_p, float f_p, float fov = 1.0);

    void Move(const Vector3D& mov);
    void Rotate(const RotationAngles& rot);

    // Преобразование геометрии для вершинных шейдеров
    // Возвращает <min_row, max_row> - границы строк, содержащих объект
    // src_vertices - N колонок (x, y, z, 1) - 3D-вектора-вершины, z_vector - z-координаты вершин
    // vecs_3d, pixels (без Clump!) - выходные данные
    std::pair<ScreenDimension, ScreenDimension> TransformGeometry(
        const SrcGeometryMatrix& src_vertices, TransformedGeometryMatrix& vecs_3d, PixelsMatrix& pixels) const;

    ScreenDimension Width()  const { return screen_width_;  }
    ScreenDimension Height() const { return screen_height_; }
    size_t PixelsCount() const { return pixels_count_; }

    WorldPoint CameraPos() const { return camera_pos_; }
    WorldPoint LightPos()  const { return light_pos_;  }
    void SetLightPos(const WorldPoint& pos) { light_pos_ = pos; }

private:
    void UpdateTransformMatrices();
    std::pair<ScreenDimension, ScreenDimension> TransformToPixels(
        const TransformedGeometryMatrix& m, PixelsMatrix& p) const;

private:
    // Преобразования геометрии
    TransformMatrix perspective_; // Считаем 1 раз
    TransformMatrix rotation_;
    TransformMatrix result_transform_;

    WorldPoint light_pos_;
    WorldPoint camera_pos_;

    // Геометрия экрана
    ScreenDimension screen_width_ = 0;
    ScreenDimension screen_height_ = 0;
    size_t pixels_count_ = 0; // w*h

    float n_ = 0; // Ближний план 
    float f_ = 0; // Дальный план
    float fov_ = 1.0;
};

typedef std::shared_ptr<RenderingGeometry> RenderingGeometryPtr;
typedef std::shared_ptr<const RenderingGeometry> RenderingGeometryConstPtr;

} // namespace plane_render