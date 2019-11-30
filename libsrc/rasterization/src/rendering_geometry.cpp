#include "rendering_geometry.hpp"

namespace plane_render {

RenderingGeometry::RenderingGeometry(ScreenDimension w, ScreenDimension h, float n_p, float f_p, float fov,
                                     const Vector3D& up) :
        up_(up),
        perspective_({}),
        result_space_({}),
        screen_width_(w), screen_height_(h), n_(n_p), f_(f_p), fov_(fov)
{
    DCHECK(w > 0 && h > 0);
    DCHECK(n_ > 0 && f_ > 0 && n_ < f_);

    // Создает __m128 перевода в пиксели
    SetToPixelsCoeffitients();

    // Считаем перспективу 1 раз
    ratio_ = static_cast<float>(screen_width_) / screen_height_;
    perspective_.rows[0] = { 1.f / std::tan(fov_), 0, 0, 0 };
    perspective_.rows[1] = { 0, ratio_ / std::tan(fov_), 0, 0 };
    perspective_.rows[2] = { 0, 0, (n_ + f_) / (n_ - f_), 2/(1/f_ - 1/n_) };
    perspective_.rows[3] = { 0, 0, 1.f, 0 };

    UpdateTransform();
}

void RenderingGeometry::LookAt(const Vector3D& pos, const Vector3D& at)
{
    camera_pos_src_ = pos;
    at_ = at;
    
    UpdateTransform();
}

/*
    float cos_t = cos(rot.theta), sin_t = sin(rot.theta);
    float sin_f = sin(rot.phi), cos_f = cos(rot.phi);
    FastVector3D e3_rot = { cos_t*sin_f, sin_t, cos_t*cos_f };
    FastVector3D e1_rot = up_.Cross(e3_rot).Normalized();
    FastVector3D e2_rot = e3_rot.Cross(e1_rot).Normalized();

    curr_rotation_matrix.rows[0] = { e1_rot.x, e1_rot.y, e1_rot.z, 0 };
    curr_rotation_matrix.rows[1] = { e2_rot.x, e2_rot.y, e2_rot.z, 0 };
    curr_rotation_matrix.rows[2] = { e3_rot.x, e3_rot.y, e3_rot.z, 0 }; // Фактически, поворотная часть at
*/
void RenderingGeometry::UpdateTransform()
{
    FastVector3D dir = static_cast<FastVector3D>(camera_pos_src_ - at_);
    if (dir.NormSq() == 0.f)
    {
        result_space_ = Matrix4::Identity();
        return;
    }
    else
        dir = dir.Normalized();
    
    FastVector3D right = up_.Cross(dir).Normalized();
    FastVector3D e2 = dir.Cross(right).Normalized();
    result_space_ = { right.x, right.y, right.z, -camera_pos_src_.Dot(right),
                      e2.x,    e2.y,    e2.z,    -camera_pos_src_.Dot(e2),
                      dir.x,   dir.y,   dir.z,   -camera_pos_src_.Dot(dir),
                      0.f,     0.f,     0.f,     1.f };

    light_pos_ = result_space_ * light_pos_src_;
}

void RenderingGeometry::MoveAt(const Vector3D& at_shift)
{
    at_ = at_ + FastVector3D(at_shift);
    UpdateTransform();
}

void RenderingGeometry::MoveCam(const Vector3D& mov)
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
    topixels_add_ = _mm_set_ps(0.f, 0.f, (-0.5f + screen_height_/2.f), (-0.5f + screen_width_/2.f));
}

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
    SetPixelPos(coords_screenspace, out_v);
}

} // namespace plane_render