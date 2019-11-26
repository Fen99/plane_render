#include "rendering_geometry.hpp"

namespace plane_render {

const FastVector3D RenderingGeometry::UP{0.f, 1.f, 0.f};

RenderingGeometry::RenderingGeometry(ScreenDimension w, ScreenDimension h, float n_p, float f_p, float fov) :
        rotation_(Matrix4::Identity()),
        screen_width_(w), screen_height_(h), n_(n_p), f_(f_p), fov_(fov)
{
    DCHECK(w > 0 && h > 0);
    DCHECK(n_ > 0 && f_ > 0 && n_ < f_);

    // Создает __m128 перевода в пиксели
    SetToPixelsCoeffitients();

    // Считаем перспективу 1 раз
    float ratio = static_cast<float>(screen_width_) / screen_height_;
    perspective_.rows[0] = { 1.f / std::tan(fov_), 0, 0, 0 };
    perspective_.rows[1] = { 0, ratio / std::tan(fov_), 0, 0 };
    perspective_.rows[2] = { 0, 0, (n_ + f_) / (n_ - f_), 2/(1/f_ - 1/n_) };
    perspective_.rows[3] = { 0, 0, 1.f, 0 };

    UpdateTransform();
}

void RenderingGeometry::LookAt(const Vector3D& pos, const RotationAngles& angles)
{
    camera_pos_src_ = pos;
    rotation_ = Matrix4::Identity();
    Rotate(angles);
}

void RenderingGeometry::UpdateTransform()
{
    // Трансляция
    Matrix4 motion = { 1, 0, 0, -camera_pos_src_.x,
                       0, 1, 0, -camera_pos_src_.y,
                       0, 0, 1, -camera_pos_src_.z,
                       0, 0, 0, 1 };
    result_space_ = rotation_ * motion;

    light_pos_ = result_space_ * light_pos_src_;
}

void RenderingGeometry::Rotate(const RotationAngles& rot)
{
    Matrix4 curr_rotation_matrix = Matrix4::Identity();

    float cos_t = cos(rot.theta), sin_t = sin(rot.theta);
    float sin_f = sin(rot.phi), cos_f = cos(rot.phi);
    FastVector3D e3_rot = { cos_t*sin_f, sin_t, cos_t*cos_f };
    FastVector3D e1_rot = UP.Cross(e3_rot).Normalized();
    FastVector3D e2_rot = e3_rot.Cross(e1_rot).Normalized();

    curr_rotation_matrix.rows[0] = { e1_rot.x, e1_rot.y, e1_rot.z, 0 };
    curr_rotation_matrix.rows[1] = { e2_rot.x, e2_rot.y, e2_rot.z, 0 };
    curr_rotation_matrix.rows[2] = { e3_rot.x, e3_rot.y, e3_rot.z, 0 };
    rotation_ = rotation_ * curr_rotation_matrix;

    UpdateTransform();
}

void RenderingGeometry::Move(const Vector3D& mov)
{
    camera_pos_src_ = camera_pos_src_ + FastVector3D(mov);
    UpdateTransform();
}

void RenderingGeometry::SetLightSrcPos(const Vector3D& pos)
{
    light_pos_src_ = pos;
    light_pos_ = result_space_ * light_pos_src_;
}

void RenderingGeometry::SetToPixelsCoeffitients()
{
    topixels_mul_ = _mm_set_ps(1.f, 1.f, screen_height_ /2.f, screen_width_/2.f);
    topixels_add_ = _mm_set_ps(0.f, 0.f, (-0.5f + screen_height_/2.f), (-0.5f + screen_width_ /2.f));
}

/*
PixelPoint RenderingGeometry::ToPixelsPoint(float ksi, float eta) const
{
    return { Clump((ScreenDimension) std::round(-1.0/2 + screen_width_ / 2.0*(ksi + 1)), 0, screen_width_-1),
             Clump((ScreenDimension) std::round(-1.0/2 + screen_height_ / 2.0*(eta + 1)), 0, screen_height_-1) };
}
*/
void RenderingGeometry::TransformGeometry(const Vector4D& src_vec4, Vertex& out_v) const
{
    // Сохраняем геометрию (для z-буффера и фрагментного шейдера)
    out_v.vertex_coords = result_space_ * src_vec4; // (xt, yt, zt, 1.0)

    // Точки с положительным и нулевым z не могут быть отображены!
    if (out_v.vertex_coords.z > -GraphicsEps)
        return;

    // Получаем (ksi, eta, dzeta) и переводим его в пиксели
    Vector4D coords4d = perspective_ * out_v.vertex_coords; // (ksi*z, eta*z, dzeta*z, z)
    FastVector3D coords_screenspace = _mm_div_ps(coords4d, _mm_set1_ps(out_v.vertex_coords.z));
    __m128 pixel_pos = _mm_add_ps(_mm_mul_ps(coords_screenspace, topixels_mul_), topixels_add_);
    pixel_pos = _mm_round_ps(pixel_pos, _MM_FROUND_TO_NEAREST_INT | _MM_FROUND_NO_EXC);
    out_v.SetPixelPos(pixel_pos);
}

} // namespace plane_render