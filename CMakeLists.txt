cmake_minimum_required(VERSION 3.24)

project(GpuRayTracer LANGUAGES C CXX CUDA)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CUDA_STANDARD 20)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

find_package(CUDAToolkit REQUIRED)

list(APPEND CMAKE_MODULE_PATH
    "${PROJECT_SOURCE_DIR}/cmake"
    "${PROJECT_SOURCE_DIR}/cmake/dependencies"
)

include(Resolver)
include(Definitions)

foreach(dep_name IN LISTS CORE_DEPENDENCIES)
    resolve_dependency(${dep_name})
endforeach()

foreach(dep_name IN LISTS APP_DEPENDENCIES)
    resolve_dependency(${dep_name})
endforeach()

add_subdirectory(Core)
add_subdirectory(App)

set(VS_FORCE_RECONFIGURE "13" CACHE STRING "noop")
