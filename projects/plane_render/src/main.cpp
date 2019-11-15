#include <iostream>

#include "sdl_adapter.hpp"

constexpr int Width = 1920;
constexpr int Height = 1080;

int main(int argc, char* argv[])
{
    plane_render::ConfigureLogger("logger.conf");
    LOG(WARNING) << "Hello, SDL!";
    
    if (argc < 2)
        throw std::invalid_argument("Mesh name should be specified as the second argument!");

    plane_render::RenderingGeometryPtr geom = std::make_shared<plane_render::RenderingGeometry>(Width, Height, 500, 1000);
    geom->Move({0, 0, 500});
    geom->SetLightPos({0, 0, 200});

    plane_render::RasterizationPipelinePtr pipeline =
        std::make_shared<plane_render::RasterizationPipeline>(geom, std::vector<std::string>{ argv[1] });

    plane_render::SDLAdapter adapter(pipeline);
    adapter.MessageLoop();
    return 0;
}