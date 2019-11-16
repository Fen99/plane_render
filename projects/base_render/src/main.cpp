#include <iostream>

#include "rasterization/fragment_shader.hpp"
#include "rasterization/vertex_shader.hpp"

#include "sdl_adapter/sdl_adapter.hpp"

constexpr int Width = 1920;
constexpr int Height = 1080;

int main(int argc, char* argv[])
{
    plane_render::ConfigureLogger("logger.conf");

    if (argc < 3)
        throw std::invalid_argument(std::string("Usage: ./")+argv[0]+" <obj_name> <ppm_name>");

    plane_render::RenderingGeometryPtr geom = std::make_shared<plane_render::RenderingGeometry>(Width, Height, 700, 1000);
    geom->Move({0, 0, 500});
    geom->SetLightPos({0, 0, 200});

    std::vector<plane_render::SceneObject> objects;
    objects.emplace_back(geom, argv[1], 100);
    objects.back().SetShaders<plane_render::VertexShader, plane_render::FragmentShader>();
    objects.back().GetFS()->LoadTexture(argv[2]);

    plane_render::RasterizationPipelinePtr pipeline =
        std::make_shared<plane_render::RasterizationPipeline>(geom, std::move(objects));

    plane_render::SDLAdapter adapter(pipeline);
    adapter.MessageLoop();
    return 0;
}
