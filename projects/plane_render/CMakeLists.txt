project(plane_render)

set(PLANE_RENDER_SRC
    src/sdl_adapter.cpp
    src/main.cpp
)
set(PLANE_RENDER_DEPENDENCIES rasterization)

build_executable(PLANE_RENDER_SRC PLANE_RENDER_DEPENDENCIES)
target_include_directories(plane_render PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(plane_render ${GSL_LIBRARY} ${SDL2_LIBRARIES})