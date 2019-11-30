project(plane_render)

set(PLANE_RENDER_SRC
    src/main.cpp
)
set(PLANE_RENDER_DEPENDENCIES sdl_adapter rasterization)

build_executable(PLANE_RENDER_SRC PLANE_RENDER_DEPENDENCIES)
