project(rasterization)
set(RASTERIZATION_SRC
    src/rendering_geometry.cpp
    src/fragment_shader.cpp
    src/texture.cpp
    src/pipeline.cpp
    src/screen_buffer.cpp
    src/rasterizer.cpp
    src/scene_object.cpp
)
set(RASTERIZATION_DEPENDENCES common threadpool)

build_static_lib(RASTERIZATION_SRC RASTERIZATION_DEPENDENCES)
