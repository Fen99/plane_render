#include "raycast.hpp"
#include "rasterization/pipeline.hpp"
#include "rasterization/fragment_shader.hpp"
#include "sdl_adapter/sdl_adapter.hpp"

#include <iostream>

using namespace plane_render;

// Облегчает преобразование
vec3 ToVec3(FastVector3D v)
{
    return { v.x, v.y, v.z };
}

vec3 ToVec3(Vector3D v)
{
    return { v.x, v.y, v.z };
}

// =================== Определяем сцену ==============================
constexpr int Width = 1920;
constexpr int Height = 1080;

sphere const sph[] =
{
    {vec3(0.f, 0.f, -1e4f),     1e4f,   vec3(0.3f, 1.f, 0.3f)},
    {vec3(0.f, 0.f, 0.3f),      0.5f,   vec3(1.f, 1.f, 1.f)}
};
int const sph_size = sizeof(sph) / sizeof(sphere);

std::vector<Vector3D> verts = // Vector3D - для совместимости
{
    {-1.f, -1.f, 0.f },
    {-1.f,  0.f, 0.f },
    {-1.f,  1.f, 0.f },
    { 0.f, -1.f, 0.f },
    { 0.f,  0.f, 0.f },
    { 0.f,  1.f, 0.f },
    { 1.f, -1.f, 0.f },
    { 1.f,  0.f, 0.f },
    { 1.f,  1.f, 0.f }
};
std::vector<size_t> inds =
{
    0, 3, 1,
    1, 3, 4,
    3, 6, 4,
    4, 6, 7,
    1, 4, 2,
    2, 4, 5,
    4, 7, 5,
    5, 7, 8
};

// =================== Шейдеры ==============================

class CustomVS : public SceneObject::VertexShader
{
protected:
    virtual void Update() override
    {
        const auto& src_coords = GetAssociatedSrcCoords();
        auto& vertices = GetAssociatedVertices();

        DCHECK(src_coords.size() == vertices.size());
        for (size_t i = 0; i < src_coords.size(); i++)
        {
            vertices[i].vertex_coords = FastVector3D{ -src_coords[i].x, -src_coords[i].y, -1.f };
            GetGeom().SetPixelPos(vertices[i].vertex_coords * (-1), vertices[i]);
        }
    }

protected:
    using SceneObject::VertexShader::VertexShader;
    friend class SceneObject; // Чтобы создавал объекты
};


class CustomFS : public FragmentShader
{
public:
    using FragmentShader::FragmentShader;

    // Инкапсулирует внешний код для фрагментного шейдера
    vec3 RawProcessFragment(const Vertex& vertex_avg) const
    {
        camera cam = { geom_->GetFov(), geom_->GetRatio(), ToVec3(geom_->CameraPosSrc()), 
                       ToVec3(geom_->GetAt()), ToVec3(geom_->GetUp()) };
        ray r = cast_from_cam(cam, { vertex_avg.vertex_coords.x, vertex_avg.vertex_coords.y } );
        intersection const I = find_intersection(r, sph, sph_size);
        if(I.idx < 0)
            return vec3(0.1f, 0.1f, 0.4f);

        vec3 const color = sph[I.idx].color;
        vec3 const pos = ray_vec(r, I.t);
        vec3 light = normalize(ToVec3(geom_->LightPos()));
        intersection const shadow = find_intersection({pos, light}, sph, sph_size);
        if(shadow.idx >= 0)
            return color * 0.3f;
        
        vec3 const norm = normalize(pos - sph[I.idx].pos);
        vec3 const halfway = normalize(light - r.dir);

        float const NH = std::max(0.f, dot(norm, halfway));
        float spec = NH;
        for(int i = 0; i < 4; i++)
            spec *= spec;
        float const diff = std::max(0.f, dot(norm, light));
        return color * (0.3f + 0.4f * diff + 0.3f * spec);
    }

    virtual Color ProcessFragment(const Vertex& vertex_avg) const override
    {
        vec3 preprocessed = RawProcessFragment(vertex_avg);
        return Color{0, 255.f*preprocessed.x, 255.f*preprocessed.y, 255.f*preprocessed.z};
    }
};

// =================== Код рисования ==============================

void Measurement(RenderingGeometry& geom, SDLAdapter& adapter)
{
    float const theta = 0.4f;
    int iter = 0;
    int const iters = 500;

    FastVector3D at = {0, 0, 0};
    while(iter < iters)
    {
        float const phi = 2.f / iters * 3.1415926 * iter++;
        FastVector3D dir{std::cos(phi) * std::cos(theta), std::sin(phi) * std::cos(theta), std::sin(theta)};
        FastVector3D campos = dir * 2.f;
        geom.LookAt(static_cast<FastVector3D>(campos+at).ToVector3D(), at.ToVector3D());
        adapter.DrawScreen();
    }
}

int main(int argc, char* argv[])
{
    ConfigureLogger("logger.conf");

    RenderingGeometryPtr geom = std::make_shared<RenderingGeometry>(Width, Height, 0.1, 20, 1, Vector3D{0.f, 0.f, 1.f});
    geom->SetLightSrcPos({1, 1, 2});

    std::vector<SceneObject> objects;
    objects.emplace_back(geom, verts, inds, 1);
    objects.back().SetShaders<CustomVS, CustomFS>();

    std::string perf_filename = std::string(argv[0]) + ".pd";
    if (argc > 1)
        perf_filename = argv[1];

    RenderProviderPtr pipeline =
        std::make_shared<RasterizationPipeline>(geom, std::move(objects), perf_filename);

    SDLAdapter adapter(pipeline);

    Measurement(*geom, adapter);
    adapter.MessageLoop();
    return 0;
}