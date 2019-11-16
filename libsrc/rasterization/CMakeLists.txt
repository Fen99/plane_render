project(rasterization)
set(RASTERIZATION_SRC
    src/rendering_geometry.cpp
    src/fragment_shader.cpp
    src/texture.cpp
    src/pipeline.cpp
    src/rasterizer.cpp
    src/scene_object.cpp
    src/vertex_shader.cpp
    src/graphics_types.cpp
)
set(RASTERIZATION_DEPENDENCES common threadpool libpnmio)

build_static_lib(RASTERIZATION_SRC RASTERIZATION_DEPENDENCES)
target_include_directories(rasterization PUBLIC ${ROOT_REPOSITORY_DIR}/third-party/eigen-git-mirror)
