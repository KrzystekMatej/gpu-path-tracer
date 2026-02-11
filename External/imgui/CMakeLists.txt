cmake_minimum_required(VERSION 3.16)

add_library(imgui STATIC
    imgui.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imgui_demo.cpp
    backends/imgui_impl_glfw.cpp
    backends/imgui_impl_opengl3.cpp
)

add_library(imgui::imgui ALIAS imgui)

target_include_directories(imgui
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}"
        "${CMAKE_CURRENT_SOURCE_DIR}/backends"
)

target_compile_definitions(imgui
    PUBLIC
        IMGUI_IMPL_OPENGL_LOADER_GLAD
)

target_link_libraries(imgui
    PUBLIC
        glfw
        OpenGL::GL
        glad::glad
)