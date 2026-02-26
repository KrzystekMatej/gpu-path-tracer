set(CORE_DEPENDENCIES
    SPDLOG
    YAML_CPP
    GLM
    ENTT
    XXHASH
    TINYOBJLOADER
    FREEIMAGE
    OPENGL
    GLAD
    GLFW
    IMGUI
    MIKKTSPACE
)

set(APP_DEPENDENCIES
)

# ---- spdlog ----
set(DEP_SPDLOG_NAME spdlog)
set(DEP_SPDLOG_FIND_PACKAGE_NAME spdlog)
set(DEP_SPDLOG_GIT_REPOSITORY https://github.com/gabime/spdlog.git)
set(DEP_SPDLOG_GIT_TAG v1.17.0)
set(DEP_SPDLOG_TARGET spdlog::spdlog)

# ---- yaml-cpp ----
set(DEP_YAML_CPP_NAME yaml-cpp)
set(DEP_YAML_CPP_FIND_PACKAGE_NAME yaml-cpp)
set(DEP_YAML_CPP_GIT_REPOSITORY https://github.com/jbeder/yaml-cpp.git)
set(DEP_YAML_CPP_GIT_TAG yaml-cpp-0.9.0)
set(DEP_YAML_CPP_TARGET yaml-cpp::yaml-cpp)

# ---- glm ----
set(DEP_GLM_NAME glm)
set(DEP_GLM_FIND_PACKAGE_NAME glm)
set(DEP_GLM_GIT_REPOSITORY https://github.com/g-truc/glm.git)
set(DEP_GLM_GIT_TAG 1.0.3)
set(DEP_GLM_TARGET glm::glm)

# ---- tinyobjloader ----
set(DEP_TINYOBJLOADER_NAME tinyobjloader)
set(DEP_TINYOBJLOADER_FIND_PACKAGE_NAME tinyobjloader)
set(DEP_TINYOBJLOADER_TARGET tinyobjloader::tinyobjloader)
set(DEP_TINYOBJLOADER_GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git)
set(DEP_TINYOBJLOADER_GIT_TAG v2.0.0rc13)
set(DEP_TINYOBJLOADER_INCLUDE_DIR
    "${PROJECT_SOURCE_DIR}/External/tinyobjloader"
)

# ---- entt ----
set(DEP_ENTT_NAME entt)
set(DEP_ENTT_FIND_PACKAGE_NAME EnTT)
set(DEP_ENTT_GIT_REPOSITORY https://github.com/skypjack/entt.git)
set(DEP_ENTT_GIT_TAG v3.16.0)
set(DEP_ENTT_TARGET EnTT::EnTT)

# ---- xxHash ----
set(DEP_XXHASH_NAME xxHash)
set(DEP_XXHASH_FIND_PACKAGE_NAME xxHash)
set(DEP_XXHASH_GIT_REPOSITORY https://github.com/Cyan4973/xxHash.git)
set(DEP_XXHASH_GIT_TAG v0.8.3)
set(DEP_XXHASH_TARGET xxHash::xxhash)

# ---- FreeImage ----
set(DEP_FREEIMAGE_NAME FreeImage)
set(DEP_FREEIMAGE_FIND_PACKAGE_NAME freeimage)
set(DEP_FREEIMAGE_TARGET freeimage::FreeImage)
set(DEP_FREEIMAGE_PKG_CONFIG_NAME freeimage)
set(DEP_FREEIMAGE_FIND_PATH_HEADER FreeImage.h)
set(DEP_FREEIMAGE_FIND_LIBRARY_NAME freeimage)

# ---- OpenGL ----
set(DEP_OPENGL_NAME OpenGL)
set(DEP_OPENGL_FIND_PACKAGE_NAME OpenGL)
set(DEP_OPENGL_TARGET OpenGL::GL)

# ---- GLAD ----
set(DEP_GLAD_NAME glad)
set(DEP_GLAD_FIND_PACKAGE_NAME glad)
set(DEP_GLAD_TARGET glad::glad)

# ---- GLFW ----
set(DEP_GLFW_NAME glfw)
set(DEP_GLFW_FIND_PACKAGE_NAME glfw3)
set(DEP_GLFW_TARGET glfw)
set(DEP_GLFW_PKG_CONFIG_NAME glfw3)
set(DEP_GLFW_GIT_REPOSITORY https://github.com/glfw/glfw.git)
set(DEP_GLFW_GIT_TAG 3.4)

# ---- ImGui ----
set(DEP_IMGUI_NAME imgui)
set(DEP_IMGUI_FIND_PACKAGE_NAME imgui)
set(DEP_IMGUI_TARGET imgui::imgui)

# ---- MikkTSpace ----
set(DEP_MIKKTSPACE_NAME mikktspace)
set(DEP_MIKKTSPACE_FIND_PACKAGE_NAME mikktspace)
set(DEP_MIKKTSPACE_TARGET mikktspace)