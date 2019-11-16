project(base_render)

set(BASE_RENDER_SRC
    src/main.cpp
)
set(BASE_RENDER_DEPENDENCIES sdl_adapter)

build_executable(BASE_RENDER_SRC BASE_RENDER_DEPENDENCIES)
