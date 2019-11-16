cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE DEBUG)
endif()

set(ROOT_REPOSITORY_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
message("Cmake on: ${ROOT_REPOSITORY_DIR}")

# C++ compliler
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED YES)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "-Wall -Wextra -pthread")
    set(CMAKE_CXX_FLAGS_DEBUG "-g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

#####################################################################
# Dependencies
#####################################################################
# SDL
find_package(SDL2 REQUIRED)

#####################################################################
# Build
#####################################################################
# Out dir (for install commands)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${ROOT_REPOSITORY_DIR}/bin/")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${ROOT_REPOSITORY_DIR}/lib/")
file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
file(MAKE_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

# Includes
set(INCLUDE_BASE_DIR include)
include_directories(${INCLUDE_BASE_DIR})

# Macro for building
macro(build_static_lib_proj src_files dependencies proj_name)
    message("Configure lib ${proj_name}")
    message("Src: ${${src_files}}")
    message("Dependencies: ${${dependencies}}")

    add_library(${proj_name} STATIC ${${src_files}})

    target_include_directories(${proj_name} PRIVATE ${ROOT_REPOSITORY_DIR}/${INCLUDE_BASE_DIR}/${proj_name})
    if (${dependencies})
        target_link_libraries(${proj_name} ${${dependencies}})
    endif()
endmacro(build_static_lib_proj)

macro(build_static_lib src_files dependencies)
    build_static_lib_proj(${src_files} ${dependencies} ${PROJECT_NAME})
endmacro(build_static_lib)

macro(build_executable src_files dependencies)
    message("Configure executable ${PROJECT_NAME}")
    message("Src: ${${src_files}}")
    message("Dependencies: ${${dependencies}}")

    add_executable(${PROJECT_NAME} ${${src_files}})

    if (${dependencies})
        target_link_libraries(${PROJECT_NAME} ${${dependencies}})
    endif()
endmacro(build_executable)

#####################################################################
# Targets
#####################################################################
# Dependencies for build
function(build_logger)
    set(build_static_lib ON CACHE BOOL "Build as static")
    add_subdirectory(third-party/easyloggingpp)
    target_compile_definitions(easyloggingpp PRIVATE ELPP_NO_LOG_TO_FILE ELPP_NO_DEFAULT_LOG_FILE)
endfunction()
build_logger()

# Libraries
add_subdirectory(libsrc/common)
add_subdirectory(libsrc/rasterization)
add_subdirectory(libsrc/threadpool)
add_subdirectory(libsrc/sdl_adapter)

# Projects (.so/.dll and executables)
add_subdirectory(projects/base_render)
