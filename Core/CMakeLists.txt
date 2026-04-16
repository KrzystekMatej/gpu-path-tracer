add_library(core_cuda_kernels STATIC)

target_sources(core_cuda_kernels PRIVATE
    "Source/Graphics/Cuda/PathTracing/Kernels.cu"
)
target_include_directories(core_cuda_kernels PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Include")
target_link_libraries(core_cuda_kernels PUBLIC CUDA::cudart)

set_target_properties(core_cuda_kernels PROPERTIES
    CUDA_SEPARABLE_COMPILATION ON
)

add_library(core STATIC)

target_sources(core PRIVATE
    "Source/Assets/Key.cpp"
    "Source/Assets/Manager.cpp"

    "Source/Capture/Recording.cpp"
    "Source/Capture/Resampling.cpp"

    "Source/ECS/SceneNodes/Background.cpp" 
    "Source/ECS/SceneNodes/Camera.cpp"
    "Source/ECS/SceneNodes/Light.cpp" 
    "Source/ECS/SceneNodes/Model.cpp" 
    "Source/ECS/SceneNodes/BuilderRegistry.cpp"
    "Source/ECS/SceneNodes/Transform.cpp"

    "Source/ECS/Systems/Capture.cpp"
    "Source/ECS/Systems/Render.cpp"
	"Source/ECS/Systems/ScriptRunner.cpp"
    "Source/ECS/Systems/Transform.cpp" 

    "Source/ECS/Scene.cpp" 

    "Source/External/TinyObjLoader.cpp"

    "Source/Graphics/Cpu/Resources/EnvironmentMap.cpp"

    "Source/Graphics/Cuda/Memory/DeviceBuffer1D.cpp"
    "Source/Graphics/Cuda/Memory/DeviceBuffer2D.cpp"
    "Source/Graphics/Cuda/Memory/DeviceQueue.cpp"
    "Source/Graphics/Cuda/Memory/SharedBuffer1D.cpp"
    "Source/Graphics/Cuda/Memory/SharedBuffer2D.cpp"
    "Source/Graphics/Cuda/Memory/SharedCounter.cpp"
    "Source/Graphics/Cuda/PathTracing/Renderer.cpp"
    "Source/Graphics/Cuda/Resources/Texture.cpp"
    "Source/Graphics/Cuda/Utils/Error.cpp"

    "Source/Graphics/Gl/Resources/Mesh.cpp"
    "Source/Graphics/Gl/Resources/Texture.cpp"
    "Source/Graphics/Gl/DynamicTexture2D.cpp"
    "Source/Graphics/Gl/Framebuffer.cpp"
    "Source/Graphics/Gl/Renderbuffer.cpp"
    "Source/Graphics/Gl/Renderer.cpp"
	"Source/Graphics/Gl/RenderTarget.cpp"
    "Source/Graphics/Gl/Shader.cpp"
    "Source/Graphics/Gl/ShaderProgram.cpp" 
    "Source/Graphics/Gl/VertexLayout.cpp" 
    "Source/Graphics/Services/SceneRenderer.cpp"

    "Source/Import/ImageLoader.cpp"
    "Source/Import/ImageUtils.cpp"
    "Source/Import/ObjLoader.cpp"
    "Source/Import/SceneLoader.cpp" 
    "Source/Import/ShaderLoader.cpp" 

    "Source/Project/Descriptor.cpp" 

    "Source/Runtime/Application.cpp"
    "Source/Runtime/Time.cpp"
    "Source/Runtime/ImGuiClient.cpp" 

    "Source/Scripts/Catalog.cpp"

    "Source/Utils/Math/TangentSpace.cpp" 
    "Source/Utils/Path.cpp"
    "Source/Utils/Yaml.cpp" 
    "Source/Utils/Text.cpp"

    "Source/Window/GlfwCallbacks.cpp"
    "Source/Window/GraphicsContext.cpp" 
    "Source/Window/NativeWindow.cpp"
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
