#include <Core/Graphics/Cuda/PathTracing/Renderer.hpp>
#include <Core/Graphics/Cuda/PathTracing/Kernels/Launchers.hpp>
#include <Core/Ecs/Transform.hpp>
#include <Core/Graphics/Ecs/Assets.hpp>
#include <Core/Graphics/Ecs/Camera.hpp>
#include <Core/Graphics/Cuda/PathTracing/Material.hpp>
#include <ranges>
#include <Core/Graphics/Cuda/Bvh/Triangle.hpp>
#include <Core/Graphics/Cuda/Utils/Glm.hpp>
#include <Core/Graphics/Cuda/Bvh/HostBvh.hpp>
#include <Core/Graphics/Cuda/Bvh/DeviceBvh.hpp>
#include <Core/Utils/Math/CoordinateSystem.hpp>
#include <Core/Graphics/Cuda/Utils/Error.hpp>
#include <cmath>
#include <Core/Graphics/Cuda/PathTracing/PathTracerDefaults.hpp>
#include <Core/Utils/Time.hpp>
#include <Core/Graphics/Cuda/PathTracing/Memory/MaterialEvalQueueViewsProvider.hpp>
#include <Core/Graphics/Cuda/Runtime/Global.hpp>
#include <Core/Graphics/Cuda/Runtime/Sync/Profiler.hpp>
#include <Core/Import/ImageIO.hpp>
#include <Core/Import/ImageUtils.hpp>
#include <Core/Utils/Path.hpp>

namespace Core::Graphics::Cuda
{
    namespace
    {
        using MaterialEvaluator =
            void (*)(
                bool,
                cudaStream_t,
                PathPoolView,
                MaterialEvalQueueView,
                Runtime::DeviceBuffer1DView<TriangleShading>,
                Runtime::DeviceBuffer1DView<Material>,
                uint32_t,
                LightSamplerView,
                ShadowRayQueueView,
                RegenQueueView,
                Runtime::DeviceBuffer1DView<float4>,
                RayQueueView);

        constexpr std::array<MaterialEvaluator, static_cast<size_t>(GlobalShadingModel::Count)> CreateMaterialEvaluators()
        {
            std::array<MaterialEvaluator, static_cast<size_t>(GlobalShadingModel::Count)> evaluators{};
            evaluators[static_cast<size_t>(GlobalShadingModel::Normal)] = &Kernels::EvaluateNormalMaterial;
            evaluators[static_cast<size_t>(GlobalShadingModel::Diffuse)] = &Kernels::EvaluateDiffuseMaterial;
            evaluators[static_cast<size_t>(GlobalShadingModel::Mirror)] = &Kernels::EvaluateMirrorMaterial;
            evaluators[static_cast<size_t>(GlobalShadingModel::Phong)] = &Kernels::EvaluatePhongMaterial;
            evaluators[static_cast<size_t>(GlobalShadingModel::Ggx)] = &Kernels::EvaluateGgxMaterial;
            evaluators[static_cast<size_t>(GlobalShadingModel::Emissive)] = &Kernels::EvaluateEmissiveMaterial;
            return evaluators;
        }

        constexpr std::array<MaterialEvaluator, static_cast<size_t>(GlobalShadingModel::Count)> MaterialEvaluators = CreateMaterialEvaluators();

        uchar4 MakeSimulationColor(uint32_t frameIndex)
        {
            const uint8_t r = static_cast<uint8_t>((frameIndex * 37u) % 256u);
            const uint8_t g = static_cast<uint8_t>((frameIndex * 73u) % 256u);
            const uint8_t b = static_cast<uint8_t>((frameIndex * 131u) % 256u);
            return make_uchar4(r, g, b, 255);
        }
        
        struct MaterialTable
        {
            std::vector<Material> materials;
            std::unordered_map<Core::Utils::Guid, uint32_t> indices;
        };

        MaterialTable BuildMaterialTable(
            const entt::registry& sceneRegistry,
            const Assets::Storage& storage,
            std::array<uint32_t, static_cast<size_t>(GlobalShadingModel::Count)>& materialCounts)
        {
            MaterialTable table;

            sceneRegistry
                .view<Core::Ecs::WorldTransform, Ecs::Mesh, Ecs::Material>()
                .each([&](const Core::Ecs::WorldTransform&, const Ecs::Mesh&, const Ecs::Material& material)
                {
                    const Core::Utils::Guid materialId = material.handle.GetId();

                    if (table.indices.contains(materialId))
                        return;

                    const auto& materialAsset = storage.Get(material.handle).value().get();
                    const uint32_t materialIndex = static_cast<uint32_t>(table.materials.size());

                    materialCounts[static_cast<size_t>(ToGlobalShadingUnchecked(materialAsset.surface))]++;

                    table.indices.emplace(materialId, materialIndex);
                    table.materials.emplace_back(Material{
                        .color = storage.Get(materialAsset.color).value().get().cuda.GetView<float4>(),
                        .specular = storage.Get(materialAsset.specular).value().get().cuda.GetView<float4>(),
                        .shininess = storage.Get(materialAsset.shininess).value().get().cuda.GetView<float>(),
                        .rma = storage.Get(materialAsset.rma).value().get().cuda.GetView<float4>(),
                        .emission = storage.Get(materialAsset.emission).value().get().cuda.GetView<float4>(),
                        .normal = storage.Get(materialAsset.normal).value().get().cuda.GetView<float4>(),
                        .ior = materialAsset.ior,
                        .transmission = materialAsset.transmission,
                    });
                });

            return table;
        }

        std::expected<Runtime::DeviceBuffer1D, Core::Utils::Error> BuildMaterialBuffer(const std::vector<Material>& materials)
        {
            Runtime::DeviceBuffer1D buffer;
            CORE_TRY_DISCARD(buffer.Allocate(static_cast<uint32_t>(materials.size()), sizeof(Material)));
            CORE_TRY_DISCARD(buffer.Upload(materials.data(), static_cast<uint32_t>(materials.size())));
            return buffer;
		}

        glm::vec3 ComputeWorldTangent(glm::mat3 model, glm::vec3 worldNormal, glm::vec3 vertexTangent)
        {
            auto makeFallbackTangent = [](const glm::vec3& normal)
            {
                const glm::vec3 axis = std::abs(normal.z) < 0.999f
                    ? glm::vec3(0.0f, 0.0f, 1.0f)
                    : glm::vec3(1.0f, 0.0f, 0.0f);

                return glm::normalize(glm::cross(axis, normal));
            };

            glm::vec3 worldTangent = model * vertexTangent;
            float tangentLengthSquared = glm::dot(worldTangent, worldTangent);

            if (tangentLengthSquared > 1e-16f)
            {
                worldTangent *= glm::inversesqrt(tangentLengthSquared);
                worldTangent -= worldNormal * glm::dot(worldNormal, worldTangent);
            }

            tangentLengthSquared = glm::dot(worldTangent, worldTangent);

            if (tangentLengthSquared > 1e-16f)
                return worldTangent * glm::inversesqrt(tangentLengthSquared);
            
            return makeFallbackTangent(worldNormal);
        }

        float3 ComputeGeometricNormal(float3 edge1, float3 edge2)
        {
            return normalize(cross(edge1, edge2));
            // safer version (in case of degenerate triangles)
            // float3 vertexNormal =
            // triangle.vertices[0].normal +
            // triangle.vertices[1].normal +
            // triangle.vertices[2].normal;

            // return Math::SafeNormalize(
            // 	cross(e1, e2),
            // 	Math::SafeNormalize(vertexNormal, make_float3(0.0f, 0.0f, 1.0f))
            // );
        }
        
        std::vector<Triangle> BuildTriangles(
            const entt::registry& sceneRegistry,
            const Assets::Storage& storage,
            const std::unordered_map<Core::Utils::Guid, uint32_t>& materialIndices)
        {
            std::vector<Triangle> triangles;
            sceneRegistry
                .view<Core::Ecs::WorldTransform, Ecs::Mesh, Ecs::Material>()
                .each([&](const Core::Ecs::WorldTransform& transform, const Ecs::Mesh& mesh, const Ecs::Material& material)
                    {
						const Cpu::Mesh& meshAsset = storage.Get(mesh.handle).value().get().cpu;
						const std::vector<Core::Graphics::Vertex>& vertices = meshAsset.GetVertices();
						const std::vector<uint32_t>& indices = meshAsset.GetIndices();

						const glm::mat4 model = transform.GetMatrix();
                        const glm::mat3 linear = glm::mat3(model);
						const glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(linear)));

                        const uint32_t materialIndex = materialIndices.at(material.handle.GetId());
                        const GlobalShadingModel shadingModel = ToGlobalShadingUnchecked(storage.Get(material.handle).value().get().surface);
                        
                        triangles.reserve(triangles.size() + indices.size() / 3);
                        
                        for (size_t i = 0; i < indices.size(); i += 3)
                        {
                            TriangleShading shadingData;
                            float3 positions[3];
                            for (size_t j = 0; j < 3; j++)
                            {
                                const Graphics::Vertex& vertex = vertices[indices[i + j]];

                                const glm::vec3 worldPosition = glm::vec3(model * glm::vec4(vertex.position, 1.0f));
                                const glm::vec3 worldNormal = glm::normalize(normal * vertex.normal);
                                const glm::vec3 worldTangent = ComputeWorldTangent(linear, worldNormal, vertex.tangent);

                                positions[j] = Utils::Glm::ToFloat3(worldPosition);
                                shadingData.normals[j] = make_float4(Utils::Glm::ToFloat3(worldNormal), 0.0f);
                                shadingData.tangents[j] = Utils::Glm::ToFloat4(glm::vec4(worldTangent, vertex.tangent.w));
                                shadingData.uvs[j] = Utils::Glm::ToFloat2(vertex.uv);
							}

                            TriangleIntersection intersectionData;
                            intersectionData.edge1 = make_float4(positions[1] - positions[0], 1.0f);
                            intersectionData.edge2 = make_float4(positions[2] - positions[0], 1.0f);
                            intersectionData.v0 = positions[0];
                            intersectionData.shadingModel = shadingModel;

                            shadingData.material = materialIndex;
                            shadingData.geometricNormal = ComputeGeometricNormal(make_float3(intersectionData.edge1), make_float3(intersectionData.edge2));

                            triangles.push_back({ intersectionData, shadingData, { positions[0], positions[1], positions[2] } });
                        }
                    });
            return triangles;
        }
        
        std::expected<AliasTable<uint32_t>, Core::Utils::Error> BuildLightSampler(const std::vector<Triangle>& triangles, const Runtime::Stream& stream)
        {
            std::vector<float> lightWeights;
            std::vector<uint32_t> triangleIndices;

            for (size_t i = 0; i < triangles.size(); i++)
            {
                if (triangles[i].intersection.shadingModel != GlobalShadingModel::Emissive && triangles[i].intersection.shadingModel != GlobalShadingModel::Normal)
                    continue;
                
                // TODO: Instead of using the area as weight of the emissive triangle, we could use the total emitted power of the triangle
                // power = (pi * area * integral over the emission texture)
                float area = 0.5f * length(cross(make_float3(triangles[i].intersection.edge1), make_float3(triangles[i].intersection.edge2)));
                lightWeights.push_back(area);
                triangleIndices.push_back(static_cast<uint32_t>(i));
            }
            
            AliasTable<uint32_t> lightSampler;
            CORE_TRY_DISCARD_CONTEXT(lightSampler.BuildSync(lightWeights, triangleIndices, stream), "Failed to build light table");
            return lightSampler;
        }

        DeviceCamera MakeDeviceCamera(const Graphics::Ecs::Camera& camera, float aspect, const Capture::MotionState& motionState)
        {
			glm::vec3 forward = motionState.rotation * Core::Utils::Math::CoordinateSystem::Forward;
			glm::vec3 right = motionState.rotation * Core::Utils::Math::CoordinateSystem::Right;
			glm::vec3 up = motionState.rotation * Core::Utils::Math::CoordinateSystem::Up;

			float halfH = std::tan(camera.fovY * 0.5f);
			float halfW = halfH * aspect;

			glm::vec3 origin = motionState.position;
			glm::vec3 horizontal = 2.0f * halfW * right;
			glm::vec3 vertical = 2.0f * halfH * up;
			glm::vec3 lowerLeftCorner = origin + forward - halfW * right - halfH * up;

			DeviceCamera deviceCamera;
			deviceCamera.origin = Utils::Glm::ToFloat3(origin);
			deviceCamera.horizontal = Utils::Glm::ToFloat3(horizontal);
			deviceCamera.vertical = Utils::Glm::ToFloat3(vertical);
			deviceCamera.lowerLeftCorner = Utils::Glm::ToFloat3(lowerLeftCorner);
			return deviceCamera;
        }
    }

    Renderer::~Renderer()
    {
        (void)StopRendering();
    }

    std::expected<void, Core::Utils::Error> Renderer::InitializeRenderingBuffers(uint32_t width, uint32_t height)
    {
        CORE_TRY_ASSIGN_CONTEXT(m_RenderStream, Runtime::Stream::Create(), "Failed to create CUDA stream for renderer");

		CORE_TRY_DISCARD_CONTEXT(m_PathPool.Allocate(PathTracerDefaults::PathPoolSize), "Failed to allocate path pool");
        
        for (auto& rayQueue : m_RayQueues)
        {
            CORE_TRY_DISCARD_CONTEXT(rayQueue.Allocate(PathTracerDefaults::PathPoolSize), "Failed to allocate ray queue");
        }
        for (auto& materialQueue : m_MaterialEvalQueues)
        {
            CORE_TRY_DISCARD_CONTEXT(materialQueue.Allocate(PathTracerDefaults::PathPoolSize), "Failed to allocate material evaluation queue");
        }
        CORE_TRY_DISCARD_CONTEXT(m_ShadowRayQueue.Allocate(PathTracerDefaults::PathPoolSize, m_RenderStream), "Failed to allocate shadow ray queue");
		CORE_TRY_DISCARD_CONTEXT(m_RegenQueue.Allocate(PathTracerDefaults::PathPoolSize), "Failed to allocate regen queue");

		CORE_TRY_DISCARD_CONTEXT(m_AccumulationBuffer.Allocate(width * height, sizeof(float4)), "Failed to allocate accumulation buffer");
        CORE_TRY_DISCARD_CONTEXT(m_Framebuffer.Allocate(width * height, sizeof(uchar4)), "Failed to allocate framebuffer");
        m_PixelGrid = { width, height, PathTracerDefaults::SamplesPerPixel, PathTracerDefaults::SamplesPerPixelAxis };
        CORE_TRY_DISCARD_CONTEXT(m_Framebuffer.GetDeviceBuffer().MemsetBytes(0), "Failed to clear framebuffer");
		CORE_TRY_DISCARD_CONTEXT(m_Framebuffer.CopyDeviceToHost(), "Failed to copy framebuffer to host");
        
        CORE_TRY_DISCARD_CONTEXT(Runtime::SynchronizeDevice(), "Failed to synchronize device after initializing rendering buffers");
		return {};
    }

    std::expected<void, Core::Utils::Error> Renderer::InitializeSceneBuffers(const entt::registry& sceneRegistry, const Assets::Storage& storage)
    {
        std::fill(m_MaterialCounts.begin(), m_MaterialCounts.end(), 0);
        MaterialTable materialTable = BuildMaterialTable(sceneRegistry, storage, m_MaterialCounts);
		CORE_TRY_CONTEXT(materialBuffer, BuildMaterialBuffer(materialTable.materials), "Failed to build material buffer");
		m_MaterialBuffer = std::move(materialBuffer);

		std::vector<Triangle> triangles = BuildTriangles(sceneRegistry, storage, materialTable.indices);
        CORE_TRY_DISCARD_CONTEXT(m_LightSampler.BuildSync(triangles, materialTable.materials, m_RenderStream), "Failed to build light sampler");
        HostBvh bvh(std::move(triangles));
        CORE_TRY_DISCARD_CONTEXT(m_Bvh.BuildFlattenSync(bvh.GetRoot(), bvh.GetDepth(), bvh.GetNodeCount(), bvh.GetTriangles()), "Failed to build BVH");
        CORE_TRY_DISCARD_CONTEXT(Runtime::SynchronizeDevice(), "Failed to synchronize after scene upload");
        spdlog::info("Scene buffers initialized - Triangles: {}, BVH Nodes: {} (depth: {}), Materials: {}", m_Bvh.GetTriangleCount(), m_Bvh.GetNodeCount(), m_Bvh.GetDepth(), materialTable.materials.size());
        return {};
    }

    std::optional<Core::Utils::Error> Renderer::PeekLastError() const
    {
        std::scoped_lock lock(m_ErrorMutex);
        return m_LastError;
    }

    std::optional<Core::Utils::Error> Renderer::ConsumeLastError()
    {
        std::scoped_lock lock(m_ErrorMutex);
        auto error = m_LastError;
        m_LastError.reset();
        return error;
    }

    std::expected<void, Core::Utils::Error> Renderer::Clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        if (IsRendering())
            return std::unexpected(Core::Utils::Error("Cannot clear renderer while rendering"));

        return RenderClear(r, g, b, a);
    }

    std::expected<void, Core::Utils::Error> Renderer::StartSimulation(
        uint32_t width,
        uint32_t height,
        const Graphics::Ecs::Camera& camera,
		std::vector<Capture::MotionState> cameraMotionStates,
		uint32_t samplesPerPixel,
		uint32_t pathDepthLimit,
        const std::filesystem::path& outputFolder,
        bool useNextEventEstimation)
    {
        width = std::min(std::max(width, PathTracerDefaults::MinFrameWidth), PathTracerDefaults::MaxFrameWidth);
        height = std::min(std::max(height, PathTracerDefaults::MinFrameHeight), PathTracerDefaults::MaxFrameHeight);

        samplesPerPixel = std::min(std::max(samplesPerPixel, PathTracerDefaults::MinSamplesPerPixel), PathTracerDefaults::MaxSamplesPerPixel);
        pathDepthLimit = std::min(std::max(pathDepthLimit, PathTracerDefaults::MinPathDepthLimit), PathTracerDefaults::MaxPathDepthLimit);
        m_OutputFolder = outputFolder;

        if (m_IsRendering.load(std::memory_order_relaxed))
            return std::unexpected(Core::Utils::Error("Renderer is already rendering"));

        if (m_RenderThread.joinable())
            m_RenderThread.join();

        {
            std::scoped_lock lock(m_ErrorMutex);
            m_LastError.reset();
        }

        if (width != m_PixelGrid.width || height != m_PixelGrid.height)
        {
            CORE_TRY_DISCARD_CONTEXT(m_Framebuffer.Allocate(width * height, sizeof(uchar4)), "Failed to allocate framebuffer");
            CORE_TRY_DISCARD_CONTEXT(m_AccumulationBuffer.Allocate(width * height, sizeof(float4)), "Failed to allocate accumulation buffer");
            CORE_TRY_DISCARD_CONTEXT(Runtime::SynchronizeDevice(), "Failed to synchronize after buffer allocation");
            m_PixelGrid.width = width;
            m_PixelGrid.height = height;
		}
		m_PixelGrid.samplesPerPixelAxis = static_cast<uint32_t>(std::sqrt(samplesPerPixel));
		m_PixelGrid.samplesPerPixel = m_PixelGrid.samplesPerPixelAxis * m_PixelGrid.samplesPerPixelAxis;
		m_PathDepthLimit = pathDepthLimit;
		m_UseNextEventEstimation = useNextEventEstimation;
		m_Camera = camera;
		m_CameraMotionStates = std::move(cameraMotionStates);

        m_DoneFrames.store(0, std::memory_order_relaxed);
		m_TotalFrames = static_cast<uint32_t>(m_CameraMotionStates.size());
		m_DoneSamples.store(0, std::memory_order_relaxed);
		m_TotalSamples = static_cast<uint64_t>(m_PixelGrid.samplesPerPixel) * static_cast<uint64_t>(width) * static_cast<uint64_t>(height);
        
        m_IsRendering.store(true, std::memory_order_relaxed);
        // SimulationLoop(std::stop_token{}, startFrame); // single-threaded version for debugging
        
        CORE_TRY(memoryInfo, Runtime::GetMemoryInfo());
        size_t usedBytes = memoryInfo.totalBytes - memoryInfo.freeBytes;
        spdlog::info("Renderer memory usage - Total: {:.2f} GB, Used: {:.2f} GB, Free: {:.2f} GB", memoryInfo.totalBytes / 1e9, usedBytes / 1e9, memoryInfo.freeBytes / 1e9);

        try
        {
            m_RenderThread = std::jthread([this](std::stop_token stopToken)
			{
				SimulationLoop(stopToken, 0);
			});
        }
        catch (...)
        {
			m_IsRendering.store(false, std::memory_order_relaxed);
            return std::unexpected(Core::Utils::Error("Failed to start render thread"));
        }

        return {};
    }

    std::expected<void, Core::Utils::Error> Renderer::ResumeSimulation(uint32_t startFrame)
    {
        if (m_IsRendering.load(std::memory_order_relaxed))
            return std::unexpected(Core::Utils::Error("Renderer is already rendering"));

        if (startFrame >= m_CameraMotionStates.size())
            return std::unexpected(Core::Utils::Error("Start frame is out of range - nothing to render"));
        
        m_DoneFrames.store(startFrame, std::memory_order_relaxed);
        m_DoneSamples.store(0, std::memory_order_relaxed);
        m_IsRendering.store(true, std::memory_order_relaxed);

        try 
        {
            m_RenderThread = std::jthread([this, startFrame](std::stop_token stopToken)
            {
                SimulationLoop(stopToken, startFrame);
            });
        }
        catch (...)
        {
            m_IsRendering.store(false, std::memory_order_relaxed);
            return std::unexpected(Core::Utils::Error("Failed to start render thread"));
        }

        return {};
    }

    std::expected<void, Core::Utils::Error> Renderer::StopRendering()
    {
        if (m_RenderThread.joinable())
        {
            m_RenderThread.request_stop();
            m_RenderThread.join();
        }

        CORE_TRY_DISCARD(Runtime::SynchronizeDevice());
        m_IsRendering.store(false, std::memory_order_relaxed);
        return {};
    }

    void Renderer::SimulationLoop(std::stop_token stopToken, uint32_t startFrame)
    {
        float aspect = static_cast<float>(m_PixelGrid.width) / static_cast<float>(m_PixelGrid.height);
        uint32_t generateCount = static_cast<uint32_t>(std::min<uint64_t>(m_TotalSamples, m_PathPool.GetPathCount()));
        const int frameDigits = static_cast<int>(std::max<std::size_t>(4, std::to_string(m_TotalFrames - 1).size()));

        for (uint32_t frameIndex = startFrame; frameIndex < m_TotalFrames; frameIndex++)
        {
            DeviceCamera camera = MakeDeviceCamera(m_Camera, aspect, m_CameraMotionStates[frameIndex]);
            std::string frameFileName = std::format("frame_{:0{}d}.png", m_DoneFrames.load(), frameDigits);
            auto result = RenderFrame(stopToken, camera, generateCount, frameFileName);
            if (!result)
            {
                (void)Runtime::SynchronizeDevice();
                std::scoped_lock lock(m_ErrorMutex);
                m_LastError = result.error();
                m_DoneSamples.store(0, std::memory_order_relaxed);
                break;
            }
            if (stopToken.stop_requested()) { break; }
			m_DoneFrames.fetch_add(1, std::memory_order_relaxed);
        }

        m_IsRendering.store(false, std::memory_order_relaxed);
    }

    std::expected<void, Core::Utils::Error> Renderer::RenderFrame(std::stop_token stopToken, DeviceCamera camera, uint32_t generateCount, std::string frameFileName)
    {
        CORE_TRY_CONTEXT(profiler, Core::Graphics::Cuda::Runtime::Profiler::Create("Path traced frame"), "Failed to create profiler");
        CORE_TRY_DISCARD_CONTEXT(profiler.StartProfiling(m_RenderStream), "Failed to start profiling");

        m_DoneSamples.store(0, std::memory_order_relaxed);

        uint32_t nextQueue = 0;
        uint32_t currentQueue = 1;
        MaterialEvalQueueViewsProvider materialQueueProvider;
        for (uint32_t i = 0; i < static_cast<uint32_t>(GlobalShadingModel::Count); i++)
        {
            materialQueueProvider.At(i) = m_MaterialEvalQueues[i].GetView();
        }
        CORE_TRY_DISCARD_CONTEXT(m_AccumulationBuffer.MemsetBytes(0, m_RenderStream), "Failed to clear accumulation buffer");
        
        CUDA_PROFILE_SECTION(profiler, m_RenderStream, "Initialize paths",
        {
            CUDA_TRY_KERNEL_DEBUG("InitializePaths",
                Kernels::InitializePaths(
                    m_RenderStream.GetRawHandle(),
                    generateCount,
                    camera,
                    m_PixelGrid,
                    m_PathPool.GetView(),
                    m_RayQueues[nextQueue].GetView()));
        });
        
        CUDA_PROFILE_SECTION(profiler, m_RenderStream, "Prepare iteration",
        {
            CUDA_TRY_KERNEL_DEBUG("PrepareIteration",
                Kernels::PrepareIteration(
                    m_RenderStream.GetRawHandle(),
                    m_RayQueues[currentQueue].GetView(),
                    m_RayQueues[nextQueue].GetView(),
                    m_ShadowRayQueue.GetView(),
                    m_RegenQueue.GetView(),
                    materialQueueProvider,
                    generateCount));
        });
        
        currentQueue = nextQueue;
        nextQueue = currentQueue ^ 1;

        uint32_t rayCount = generateCount;
        uint64_t launchedSampleCount = generateCount;

        bool frameCompleted = false;

        while (!frameCompleted)
        {
            CUDA_PROFILE_SECTION(profiler, m_RenderStream, "Intersect rays",
            {
                CUDA_TRY_KERNEL_DEBUG("IntersectRaysWithScene",
                    Kernels::IntersectRaysWithScene(
                        m_RenderStream.GetRawHandle(),
                        rayCount,
                        m_RayQueues[currentQueue].GetView(),
                        m_Bvh.GetIntersectionView(),
                        m_Bvh.GetDepth(),
                        materialQueueProvider,
                        m_RegenQueue.GetView()));
            });

            CORE_TRY_DISCARD_CONTEXT(m_RenderStream.Synchronize(), "Failed to synchronize after ray-scene intersection");
            
            CUDA_PROFILE_SECTION(profiler, m_RenderStream, "Evaluate materials",
            {
                for (uint32_t i = 0; i < static_cast<uint32_t>(GlobalShadingModel::Count); i++)
                {
                    if (m_MaterialCounts[i] == 0) continue;
                    MaterialEvaluator evaluator = MaterialEvaluators[i];

                    CUDA_TRY_KERNEL_DEBUG("EvaluateMaterial", 
                        evaluator(
                            m_UseNextEventEstimation,
                            m_RenderStream.GetRawHandle(),
                            m_PathPool.GetView(),
                            materialQueueProvider.At(i),
                            m_Bvh.GetShadingView(),
                            m_MaterialBuffer.GetView<Material>(),
                            m_PathDepthLimit,
                            m_LightSampler.GetView(),
                            m_ShadowRayQueue.GetView(),
                            m_RegenQueue.GetView(),
                            m_AccumulationBuffer.GetView<float4>(),
                            m_RayQueues[nextQueue].GetView()));   
                }
            });
            
            if (m_UseNextEventEstimation)
            {
                CUDA_PROFILE_SECTION(profiler, m_RenderStream, "Intersect shadow rays",
                {
                    CUDA_TRY_KERNEL_DEBUG("IntersectShadowRaysWithScene",
                        Kernels::IntersectShadowRaysWithScene(
                            m_RenderStream.GetRawHandle(),
                            m_ShadowRayQueue.GetView(),
                            m_Bvh.GetIntersectionView(),
                            m_Bvh.GetDepth(),
                            m_PathPool.GetView(),
                            m_AccumulationBuffer.GetView<float4>()));
                });
            }

            uint32_t remainingCount = static_cast<uint32_t>(std::min<uint64_t>(m_TotalSamples - launchedSampleCount, m_PathPool.GetPathCount()));
            CUDA_PROFILE_SECTION(profiler, m_RenderStream, "Regenerate paths",
            {
                CUDA_TRY_KERNEL_DEBUG("RegeneratePaths", 
                    Kernels::RegeneratePaths(
                        m_RenderStream.GetRawHandle(),
                        remainingCount,
                        m_RegenQueue.GetView(),
                        launchedSampleCount,
                        camera,
                        m_PixelGrid,
                        m_PathPool.GetView(),
                        m_RayQueues[nextQueue].GetView()));
            });

            CUDA_PROFILE_SECTION(profiler, m_RenderStream, "Sync counters",
            {
                CORE_TRY_CONTEXT(regenQueueSize, m_RegenQueue.GetCounter().SyncFromDevice(m_RenderStream), "Failed to synchronize regen queue counter");
                uint32_t regenerateCount = std::min(regenQueueSize, remainingCount);
                uint32_t nextRayCount = rayCount - regenQueueSize + regenerateCount;

                if (nextRayCount == 0)
                {
                    frameCompleted = true;
                    m_DoneSamples.store(m_TotalSamples, std::memory_order_relaxed);
                    break;
                }

                if (stopToken.stop_requested()) return {};

                CUDA_PROFILE_SECTION(profiler, m_RenderStream, "Prepare iteration",
                {
                    CUDA_TRY_KERNEL_DEBUG("PrepareIteration", 
                        Kernels::PrepareIteration(
                            m_RenderStream.GetRawHandle(),
                            m_RayQueues[currentQueue].GetView(),
                            m_RayQueues[nextQueue].GetView(),
                            m_ShadowRayQueue.GetView(),
                            m_RegenQueue.GetView(),
                            materialQueueProvider,
                            nextRayCount));
                });
                
                currentQueue = nextQueue;
                nextQueue = currentQueue ^ 1;
                rayCount = nextRayCount;
                launchedSampleCount += regenerateCount;

                m_DoneSamples.fetch_add(regenerateCount, std::memory_order_relaxed);
            });
        }
        
        CUDA_PROFILE_SECTION(profiler, m_RenderStream, "Postprocess",
        {
            CUDA_TRY_KERNEL_DEBUG("PostprocessAccumulatedRadiance",
                Kernels::PostprocessAccumulatedRadiance(
                    m_RenderStream.GetRawHandle(),
                    m_AccumulationBuffer.GetView<float4>(),
                    1.0f / static_cast<float>(m_PixelGrid.samplesPerPixel),
                    m_Framebuffer.GetDeviceView<uchar4>()));
        });
        
        CUDA_PROFILE_SECTION(profiler, m_RenderStream, "Copy frame to host",
        {
            std::scoped_lock lock(m_FrameMutex);
            CORE_TRY_DISCARD(m_Framebuffer.CopyDeviceToHost(m_RenderStream));
            CORE_TRY_DISCARD(m_RenderStream.Synchronize());
        });

        CORE_TRY_DISCARD_CONTEXT(profiler.StopProfiling(m_RenderStream), "Failed to stop profiling");
        profiler.LogResults();
        
        CORE_TRY_DISCARD_CONTEXT(SaveRenderedFrame(m_OutputFolder / frameFileName), "Failed to save rendered frame");
        return {};
    }

    std::expected<void, Core::Utils::Error> Renderer::SaveRenderedFrame(const std::filesystem::path& path)
    {
        CORE_TRY_DISCARD(Core::Utils::Path::EnsureDirectoryExists(path.parent_path()));

        uint8_t* frameDataHostBegin = reinterpret_cast<uint8_t*>(m_Framebuffer.GetHostData());
        std::vector<uint8_t> frameData;

        {
            std::scoped_lock lock(m_FrameMutex);
            frameData.assign(frameDataHostBegin, frameDataHostBegin + m_Framebuffer.GetSize() * m_Framebuffer.GetElementSize());
        }
        
        Import::Image image{
            .width = m_PixelGrid.width,
            .height = m_PixelGrid.height,
            .format = {
                .layout = ChannelLayout::RGBA,
                .componentType = ComponentType::UInt8,
                .colorSpace = ColorSpace::SRGB
            },
            .data = std::move(frameData)
        };

        CORE_TRY_DISCARD_CONTEXT(Import::SaveImage(path, Import::FlipVertically(image)), "Failed to save rendered image");
        spdlog::info("Saved rendered frame {} to '{}'", m_DoneFrames.load(), path.string());
        return {};
    }

    std::expected<void, Core::Utils::Error> Renderer::RenderClear(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        const uchar4 clearColor = make_uchar4(r, g, b, a);

        CUDA_TRY_KERNEL_DEBUG("Clear", Kernels::Clear(m_RenderStream.GetRawHandle(), clearColor, m_Framebuffer.GetDeviceView<uchar4>()));

        std::scoped_lock lock(m_FrameMutex);
        CORE_TRY_DISCARD(m_Framebuffer.CopyDeviceToHost(m_RenderStream));
        CORE_TRY_DISCARD(m_RenderStream.Synchronize());
        return {};
    }
}
