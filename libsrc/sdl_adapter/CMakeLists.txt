project(sdl_adapter)
set(SDL_ADAPTER_SRC
    src/sdl_adapter.cpp
)
set(SDL_ADAPTER_DEPENDENCES rasterization ${SDL2_LIBRARIES})

build_static_lib(SDL_ADAPTER_SRC SDL_ADAPTER_DEPENDENCES)
target_include_directories(sdl_adapter PRIVATE ${SDL2_INCLUDE_DIRS})
