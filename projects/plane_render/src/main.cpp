#include <iostream>

#include "rasterization/fragment_shader.hpp"
#include "rasterization/pipeline.hpp"

#include "sdl_adapter/sdl_adapter.hpp"

constexpr int Width = 1920;
constexpr int Height = 1080;

using namespace plane_render;

class SkyboxFS : public FragmentShader
{
public:
    using FragmentShader::FragmentShader;

    virtual Color ProcessFragment(const Vertex& vertex_avg) const override
    {
        return texture_.GetPoint(vertex_avg.texture_coords);
    }
};

void Measurement(RenderingGeometry& geom, SDLAdapter& adapter)
{
    float const theta = 0.4f;
    int iter = 0;
    int const iters = 2000;

    FastVector3D at{0.f, 0.f, 0.f};
    while(iter < iters)
    {
        float const phi = 2.f / iters * 3.1415926f * iter++;
        FastVector3D dir{std::sin(phi) * std::cos(theta), std::sin(theta), std::cos(phi) * std::cos(theta)};
        FastVector3D campos = dir * 15.f;
        geom.LookAt(static_cast<FastVector3D>(campos + at).ToVector3D(), at.ToVector3D());
        adapter.DrawScreen();
    }
}

int main(int argc, char* argv[])
{
    ConfigureLogger("logger.conf");

    if (argc < 5)
        throw std::invalid_argument(std::string("Usage: ./")+argv[0]+" <obj_name> <ppm_name>"
                                                                     " <skybox_obj_name> <skybox_ppm_name>"
                                                                     " [ <perf_filename> ]");

    RenderingGeometryPtr geom = std::make_shared<RenderingGeometry>(Width, Height, 0.1, 1050, 1);
    geom->SetLightSrcPos({1, 1, 3});

    std::vector<SceneObject> objects;
    objects.emplace_back(geom, argv[1], 1.f, 5);
    objects[0].SetShaders<SceneObject::VertexShader, FragmentShader>();
    objects[0].GetFS()->LoadTexture(argv[2]);

    objects.emplace_back(geom, argv[3], 1000, 1);
    objects[1].SetShaders<SceneObject::VertexShader, SkyboxFS>();
    objects[1].GetFS()->LoadTexture(argv[4]);

    std::string perf_filename = std::string(argv[0]) + ".pd";
    if (argc > 5)
        perf_filename = argv[5];

    RenderProviderPtr pipeline =
        std::make_shared<RasterizationPipeline>(geom, std::move(objects), perf_filename);

    SDLAdapter adapter(pipeline);

    Measurement(*geom, adapter);
    adapter.MessageLoop();
    return 0;
}
