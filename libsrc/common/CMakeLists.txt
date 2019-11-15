project(common)
set(COMMON_SRC
    src/logger.cpp
    src/math_simple.cpp
)
set(COMMON_DEPENDENCES easyloggingpp)

build_static_lib(COMMON_SRC COMMON_DEPENDENCES)
target_include_directories(common PUBLIC ${ROOT_REPOSITORY_DIR}/third-party/easyloggingpp/src)