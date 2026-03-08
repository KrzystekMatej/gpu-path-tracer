add_library(core_cuda_kernels INTERFACE) # STATIC later

#target_sources(core_cuda_kernels PRIVATE ...)
target_include_directories(core_cuda_kernels INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/Include")# PUBLIC later
target_link_libraries(core_cuda_kernels INTERFACE CUDA::cudart) # PUBLIC later

set_target_properties(core_cuda_kernels PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
)

add_library(core STATIC)

target_sources(core PRIVATE
    "Source/App/Application.cpp"
    "Source/App/Time.cpp"
    "Source/Assets/Manager.cpp"
    "Source/Assets/Key.cpp"
    "Source/Scripts/Catalog.cpp"
    "Source/ECS/Systems/Render.cpp"
	"Source/ECS/Systems/ScriptRunner.cpp"
    "Source/ECS/Resolvers/SceneResolverRegistry.cpp"
    "Source/ECS/Resolvers/Camera.cpp"
    "Source/ECS/Resolvers/Model.cpp" 
    "Source/ECS/Resolvers/Transform.cpp"
    "Source/ECS/Scene.cpp" 
    "Source/Events/EventDispatcher.cpp"
    "Source/External/TinyObjLoader.cpp"
    "Source/Graphics/Cpu/Resources/EnvironmentMap.cpp"
    "Source/Graphics/Gl/Resources/Texture.cpp"
    "Source/Graphics/Gl/Resources/Mesh.cpp"
    "Source/Graphics/Gl/VertexLayout.cpp" 
    "Source/IO/ObjLoader.cpp"
    "Source/IO/ImageLoader.cpp"
    "Source/IO/Renderer.cpp"
    "Source/Utils/Math/TangentSpace.cpp" 
    "Source/Utils/Text.cpp"
    "Source/Window/Window.cpp"
    "Source/Window/GraphicsContext.cpp" 
    "Source/Window/GLFWCallbacks.cpp"
    "Source/Project/Project.cpp" 
    "Source/IO/SceneLoader.cpp" 
    "Source/Utils/Path.cpp"
    "Source/Utils/Yaml.cpp" 
)

target_include_directories(core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include")
target_link_libraries(core PUBLIC core_cuda_kernels)

foreach(dep_name IN LISTS CORE_DEPENDENCIES)
    if(DEP_${dep_name}_TARGET AND TARGET ${DEP_${dep_name}_TARGET})
        target_link_libraries(core PUBLIC ${DEP_${dep_name}_TARGET})
    elseif(DEP_${dep_name}_INCLUDE_DIR)
        target_include_directories(core PUBLIC ${DEP_${dep_name}_INCLUDE_DIR})
    endif()
endforeach()
