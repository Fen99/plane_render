#pragma once
#include "geom.hpp"

struct sphere
{
    vec3 pos;
    float R;

    vec3 color;
};
struct ray
{
    vec3 pos;
    vec3 dir;
};
inline vec3 ray_vec(ray const &r, float const t) noexcept
{
    return r.pos + r.dir * t;
}
inline float intersect_sphere(sphere const &s, ray const &r) noexcept
{
    vec3 const rp = r.pos - s.pos;
    float const ra_rp = dot(r.dir, rp);
    if(ra_rp > 0.f)
        return 0.f;
    float const d = ra_rp * ra_rp - dot(rp, rp) + s.R * s.R;
    if(d < 0.f)
        return 0.f;
    return -ra_rp - sqrtf(d);
}

struct camera
{
    float fov;
    float ratio;
    vec3 pos;
    vec3 at;
    vec3 up;
};
inline ray cast_from_cam(camera const &cam, vec2 const &r) noexcept
{
    vec3 const n = normalize(cam.at - cam.pos);
    vec3 const right = cross(n, cam.up);
    vec3 const up = cross(right, n);
    vec3 const x = normalize(right) * (cam.fov * r.x * cam.ratio);
    vec3 const y = normalize(up) * (cam.fov * r.y);
    return ray
    {
        cam.pos,
        normalize(n + x + y)
    };
}
struct intersection
{
    float t;
    int idx;
};
inline intersection find_intersection(ray const &r, sphere const *sph, int sph_sz) noexcept
{
    float depth = 1e6;
    float const dmin = 1e-3;
    int idx = sph_sz;
    for(int i = 0; i < sph_sz; i++)
    {
        float const d = intersect_sphere(sph[i], r);
        if(d > dmin && d < depth)
        {
            depth = d;
            idx = i;
        }
    }
    return intersection{depth, idx == sph_sz ? -1 : idx};
}
