#pragma once

#include "rasterization/pipeline.hpp"

#include <memory>

struct SDL_Window;
struct SDL_Surface;
struct SDL_Renderer;
struct SDL_Texture;

namespace plane_render {

class SDLAdapter
{
public:
    SDLAdapter(const RasterizationPipelinePtr& pipeline);
    SDLAdapter(const SDLAdapter&) = delete;
    SDLAdapter& operator=(const SDLAdapter&) = delete;

    void MessageLoop();
    ~SDLAdapter();

private:
    void DrawScreen();

private:
    RasterizationPipelinePtr pipeline_;

    // Окно для рисования
    SDL_Window* window_;
    SDL_Surface* surface_;
    SDL_Renderer* renderer_;

    // Буфер, куда будем отображать пиксели
    SDL_Texture* buffer_;
};

} // namespace plane_render
