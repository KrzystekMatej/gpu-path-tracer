#include <FreeImage.h>

#include "TestApp.hpp"

#include <spdlog/spdlog.h>
#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include <tiny_obj_loader.h>

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <string>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

int main()
{
    spdlog::info("Starting App...");
    TestApp app;
    app.Run();

    YAML::Node node = YAML::Load("key: value");
    spdlog::info("YAML value = {}", node["key"].as<std::string>());

    glm::vec3 v{1.0f, 2.0f, 3.0f};
    (void)v;

    entt::registry registry;
    auto entity = registry.create();
    (void)entity;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warning;
    std::string error;
    (void)attrib;
    (void)shapes;
    (void)materials;
    (void)warning;
    (void)error;

    FreeImage_Initialise(FALSE);

    FREE_IMAGE_FORMAT fmt = FreeImage_GetFileType("dummy.png", 0);
    (void)fmt;

    const char* version = FreeImage_GetVersion();
    (void)version;

    FreeImage_DeInitialise();

    if (!glfwInit())
        return 1;

    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(64, 64, "smoke", nullptr, nullptr);
    if (!window)
    {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);

    int glad_ok = gladLoaderLoadGL();
    if (!glad_ok)
    {
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }
    
    const unsigned char* gl_version = glGetString(GL_VERSION);
    (void)gl_version;
    
    IMGUI_CHECKVERSION();
    ImGui::CreateContext(); 
    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Smoke");
    ImGui::Text("ImGui OK");
    ImGui::End();

    ImGui::Render();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    spdlog::info("Shut down.");
    return 0;
}
