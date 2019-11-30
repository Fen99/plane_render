project(ray_tracing)

set(RAY_TRACING_SRC
    src/main.cpp
)
set(RAY_TRACING_DEPENDENCIES sdl_adapter rasterization)

build_executable(RAY_TRACING_SRC RAY_TRACING_DEPENDENCIES)
