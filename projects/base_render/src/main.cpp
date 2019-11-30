#include <iostream>

#include "rasterization/fragment_shader.hpp"
#include "rasterization/pipeline.hpp"

#include "sdl_adapter/sdl_adapter.hpp"

constexpr int Width = 1920;
constexpr int Height = 1080;

using namespace plane_render;

void Measurement(RenderingGeometry& geom, SDLAdapter& adapter)
{
    float const theta = 0.4f;
    int iter = 0;
    int const iters = 2000;

    FastVector3D at{0.f, 1.f, 0.f};
    while(iter < iters)
    {
        float const phi = 2.f / iters * 3.1415926 * iter++;
        FastVector3D dir{std::sin(phi) * std::cos(theta), std::sin(theta), std::cos(phi) * std::cos(theta)};
        FastVector3D campos = dir * 2.f;
        geom.LookAt(static_cast<FastVector3D>(campos + at).ToVector3D(), at.ToVector3D());
        adapter.DrawScreen();
    }
}

int main(int argc, char* argv[])
{
    ConfigureLogger("logger.conf");

    if (argc < 3)
        throw std::invalid_argument(std::string("Usage: ./")+argv[0]+" <obj_name> <ppm_name> [ <perf_filename> ]");

//    RenderingGeometryPtr geom = std::make_shared<::RenderingGeometry>(Width, Height, 700, 1000);
//    geom->Move({0, 0, 500});
//    geom->SetLightPos({100, 100, 300});
    RenderingGeometryPtr geom = std::make_shared<RenderingGeometry>(Width, Height, 0.1, 20, 1);
    geom->SetLightSrcPos({1, 1, 3});

    std::vector<SceneObject> objects;
    objects.emplace_back(geom, argv[1], 1.0, 1000);
    objects.back().SetShaders<SceneObject::VertexShader, FragmentShader>();
    objects.back().GetFS()->LoadTexture(argv[2]);

    std::string perf_filename = std::string(argv[0]) + ".pd";
    if (argc > 3)
        perf_filename = argv[3];

    RenderProviderPtr pipeline =
        std::make_shared<RasterizationPipeline>(geom, std::move(objects), perf_filename);

    SDLAdapter adapter(pipeline);

    Measurement(*geom, adapter);
    adapter.MessageLoop();
    return 0;
}
