#include "sdl_adapter.hpp"

#include "common/logger.hpp"
#include "SDL.h"

#define CHECK_SDL(x) CHECK(x) << std::string("SDL error: ") + SDL_GetError()
#define CHECK_SDL_NOTNULL(x) CHECK_SDL((x) != nullptr)
#define CHECK_SDL_NULL(x) CHECK_SDL((x) == 0)

namespace plane_render {

SDLAdapter::SDLAdapter(const RenderProviderPtr& pipeline) :
        provider_(pipeline)
{
    CHECK_SDL(SDL_Init(SDL_INIT_VIDEO) == 0);

    CHECK_SDL_NOTNULL(window_ = 
        SDL_CreateWindow("Rasterization", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                         provider_->ScreenWidth(), provider_->ScreenHeight(), 0));
    CHECK_SDL_NOTNULL(surface_ = SDL_GetWindowSurface(window_));
    CHECK_SDL_NOTNULL(renderer_ = SDL_CreateSoftwareRenderer(surface_));

    CHECK_SDL_NOTNULL(buffer_ = 
        SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_BGRA8888, SDL_TEXTUREACCESS_STREAMING,
                          provider_->ScreenWidth(), provider_->ScreenHeight()));

    DrawScreen();
}

void SDLAdapter::DrawScreen()
{
    provider_->Update();

    int* pixels = nullptr;
    int pitch = 0;
    CHECK_SDL_NULL(SDL_LockTexture(buffer_, NULL, (void**)&pixels, &pitch));

    memcpy(pixels, provider_->GetPixels(), provider_->GetBufferSize());
    //memcpy(pixels, provider_->GetObjects()[0].GetFS()->GetTexture()->GetPixels(), provider_->GetBufferSize());

    SDL_UnlockTexture(buffer_);
    CHECK_SDL_NULL(SDL_RenderCopy(renderer_, buffer_, NULL, NULL));
    CHECK_SDL_NULL(SDL_UpdateWindowSurface(window_));
}

void SDLAdapter::MessageLoop()
{
    constexpr float speed_rot = 100; // pix / rad
    constexpr float speed_keys_move = 0.1;
    constexpr float mouse_roll_speed = 0.2;

    bool done = false;
    while (!done)
    {
        SDL_Event e;
        bool need_redraw = false;

        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
                case SDL_QUIT:
                    done = 1;
                    return;

                case SDL_MOUSEMOTION:
                    if (e.motion.state & SDL_BUTTON_LMASK)
                    {
                        if (e.motion.xrel == 0 && e.motion.yrel == 0)
                            continue;
                        float dphi = e.motion.xrel / speed_rot;
                        float dtheta = e.motion.yrel / speed_rot;
                        provider_->MoveAt(std::sin(dphi) * std::cos(dtheta), std::sin(dtheta), std::cos(dphi) * std::cos(dtheta));
                        need_redraw = true;
                    }
                    break;

                case SDL_KEYDOWN:
                    switch (e.key.keysym.sym)
                    {
#define MOVING_EVENT(event_macro, dx, dy, dz) case event_macro: \
                                                  provider_->MoveCam(dx, dy, dz); \
                                                  need_redraw = true; \
                                                  break;
                        MOVING_EVENT(SDLK_UP, 0, -speed_keys_move, 0);
                        MOVING_EVENT(SDLK_DOWN, 0, speed_keys_move, 0);
                        MOVING_EVENT(SDLK_RIGHT, speed_keys_move, 0, 0);
                        MOVING_EVENT(SDLK_LEFT, -speed_keys_move, 0, 0);
                        MOVING_EVENT(SDLK_z, 0, 0, -mouse_roll_speed);
                        MOVING_EVENT(SDLK_x, 0, 0, mouse_roll_speed);
#undef MOVING_EVENT
                    }
                    break;

                case SDL_MOUSEWHEEL:
                    if (e.wheel.y > 0)
                        provider_->MoveCam(0, 0, -mouse_roll_speed);
                    else
                        provider_->MoveCam(0, 0, mouse_roll_speed);
                    need_redraw = true;
                    break;
            }

            if (need_redraw)
                DrawScreen();
        }
    }
}

SDLAdapter::~SDLAdapter()
{
    SDL_Quit();
}

} // namespace plane_render