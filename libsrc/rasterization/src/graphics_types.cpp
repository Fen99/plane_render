#include "graphics_types.hpp"

#include <cmath>
#include <pmmintrin.h>

namespace plane_render {

Vertex Vertex::operator+(const Vertex& vp_second) const
{
    Vertex result = {_mm_add_ps(vfast.v1, vp_second.vfast.v1), _mm_add_ps(vfast.v2, vp_second.vfast.v2)};
    return result;
//     return { {position.x + vp_second.position.x, position.y + vp_second.position.y, position.z + vp_second.position.z }, 
//              { properties.texture_coords.x + vp_second.properties.texture_coords.x, properties.texture_coords.y + vp_second.properties.texture_coords.y},
//              { properties.normal.x + vp_second.properties.normal.x, properties.normal.y + vp_second.properties.normal.y, properties.normal.z + vp_second.properties.normal.z} };
}

Vertex Vertex::operator*(float val) const
{
    __m128 val_bc = _mm_set_ps1(val);
    Vertex result = { _mm_mul_ps(vfast.v1, val_bc), _mm_mul_ps(vfast.v2, val_bc) };
    return result;
//     return {{ position.x*val, position.y * val, position.z * val }, { properties.texture_coords.x * val, properties.texture_coords.y * val },
//             { properties.normal.x * val, properties.normal.y * val, properties.normal.z * val }};
}

bool BaricentricCoords::IsValid() const
{
    return a >= 0 && b >= 0 && c >= 0;
}

} // namespace plane_render