cmake_minimum_required(VERSION 3.24)

project(GpuRayTracer LANGUAGES C CXX CUDA)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CUDA_STANDARD 20)
set(CMAKE_CUDA_STANDARD_REQUIRED ON)

if (MSVC)
    # Enable warning level 4
    add_compile_options("$<$<COMPILE_LANGUAGE:C,CXX>:/W4>")
    # Enable ASan in debug builds
    string(REPLACE "/RTC1" "" CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    string(REPLACE "/RTC1" "" CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG}")

    add_compile_options("$<$<AND:$<CONFIG:Debug>,$<COMPILE_LANGUAGE:C,CXX>>:/fsanitize=address>")
    
    add_link_options("$<$<CONFIG:Debug>:/fsanitize=address>")
    add_link_options("$<$<CONFIG:Debug>:/INCREMENTAL:NO>")
endif()

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

set(VS_FORCE_RECONFIGURE "20" CACHE STRING "noop")
