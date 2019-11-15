#include "rendering_geometry.hpp"

namespace plane_render {

RenderingGeometry::RenderingGeometry(ScreenDimension w, ScreenDimension h, float n_p, float f_p) :
        screen_width_(w), screen_height_(h), n_(n_p), f_(f_p), pixels_count_(w*h),
        rotation_(TransformMatrix::Identity())
{
    CHECK(w > 0 && h > 0);
    CHECK(n_ > 0 && f_ > 0 && n_ < f_);

    // Считаем перспективу 1 раз
    perspective_.col(0) << n_ / screen_width_, 0, 0, 0;
    perspective_.col(1) << 0, n_ / screen_height_, 0, 0;
    perspective_.col(2) << 0, 0, (n_ + f_) / (n_ - f_), 1;
    perspective_.col(3) << 0, 0, 2/(1/f_ - 1/n_), 0;

    UpdateTransformMatrices();
}

void RenderingGeometry::UpdateTransformMatrices()
{
    // Трансляция
    TransformMatrix motion = TransformMatrix::Identity();
    motion.col(3) << -camera_pos_.x, -camera_pos_.y, -camera_pos_.z, 1;
    result_transform_ = perspective_ * motion * rotation_;
}

void RenderingGeometry::Rotate(const RotationAngles& rot)
{
    TransformMatrix curr_rotation_matrix = TransformMatrix::Identity();

    float cos_t = cos(rot.theta), sin_t = sin(rot.theta);
    float sin_f = sin(rot.phi), cos_f = cos(rot.phi);
    Vector3D up = { 0, 1, 0 };
    Vector3D e3_rot = { cos_t*sin_f, sin_t, cos_t*cos_f };
    Vector3D e1_rot = up.Cross(e3_rot).Normalized();
    Vector3D e2_rot = e3_rot.Cross(e1_rot).Normalized();

    curr_rotation_matrix.row(0) << e1_rot.x, e1_rot.y, e1_rot.z, 0;
    curr_rotation_matrix.row(1) << e2_rot.x, e2_rot.y, e2_rot.z, 0;
    curr_rotation_matrix.row(2) << e3_rot.x, e3_rot.y, e3_rot.z, 0;
    rotation_ = rotation_ * curr_rotation_matrix;

    UpdateTransformMatrices();
}

void RenderingGeometry::Move(const Vector3D& mov)
{
    camera_pos_ = camera_pos_ + mov;
    UpdateTransformMatrices();
}

/*
PixelPoint RenderingGeometry::ToPixelsPoint(float ksi, float eta) const
{
    return { Clump((ScreenDimension) std::round(-1.0/2 + screen_width_ / 2.0*(ksi + 1)), 0, screen_width_-1),
             Clump((ScreenDimension) std::round(-1.0/2 + screen_height_ / 2.0*(eta + 1)), 0, screen_height_-1) };
}
*/

std::pair<ScreenDimension, ScreenDimension> RenderingGeometry::TransformToPixels(
    const TransformedGeometryMatrix& m, PixelsMatrix& p) const
{
    p.row(0) = (screen_width_ /2.f*m.row(0).array() + (-0.5f + screen_width_ /2.f)).round().cast<ScreenDimension>();
    p.row(1) = (screen_height_/2.f*m.row(1).array() + (-0.5f + screen_height_/2.f)).round().cast<ScreenDimension>();
    return { Clump(p.row(1).minCoeff(), 0, screen_height_-1), Clump(p.row(1).maxCoeff(), 0, screen_height_-1) };
}

std::pair<ScreenDimension, ScreenDimension> RenderingGeometry::TransformGeometry(
        const SrcGeometryMatrix& src_vertices, TransformedGeometryMatrix& vecs_3d, PixelsMatrix& pixels) const
{
    vecs_3d = std::move((result_transform_ * src_vertices).colwise().hnormalized());
    return TransformToPixels(vecs_3d, pixels);
}

} // namespace plane_render