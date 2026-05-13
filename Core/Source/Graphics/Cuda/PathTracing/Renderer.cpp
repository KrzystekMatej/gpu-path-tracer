#include <Core/Graphics/Cuda/PathTracing/Renderer.hpp>
#include <Core/Graphics/Cuda/PathTracing/Kernels.hpp>
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

namespace Core::Graphics::Cuda
{
	constexpr size_t PathPoolSize = 1'000'000;

    namespace
    {
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
            const Assets::Storage& storage)
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
                    
                    table.indices.emplace(materialId, materialIndex);
                    table.materials.emplace_back(Material{
                        .shadingModel = ToGlobalShadingUnchecked(materialAsset.surface),
                        .albedo = storage.Get(materialAsset.albedo).value().get().cuda.GetView<float4>(),
                        .specular = storage.Get(materialAsset.specular).value().get().cuda.GetView<float4>(),
                        .shininess = storage.Get(materialAsset.shininess).value().get().cuda.GetView<float>(),
                        .roughness = storage.Get(materialAsset.roughness).value().get().cuda.GetView<float>(),
                        .metallic = storage.Get(materialAsset.metallic).value().get().cuda.GetView<float>(),
                        .ao = storage.Get(materialAsset.ao).value().get().cuda.GetView<float>(),
                        .emission = storage.Get(materialAsset.emission).value().get().cuda.GetView<float4>(),
                        .normal = storage.Get(materialAsset.normal).value().get().cuda.GetView<float4>(),
                    });
                });

            return table;
        }

        std::expected<Memory::DeviceBuffer1D, Core::Utils::Error> BuildMaterialBuffer(const std::vector<Material>& materials)
        {
            Memory::DeviceBuffer1D buffer;
            CORE_TRY_VOID(buffer.Allocate(materials.size(), sizeof(Material)));
            CORE_TRY_VOID(buffer.UploadSync(materials.data(), materials.size()));
            return buffer;
		}

        std::vector<Triangle> BuildTriangleList(
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
						const glm::mat4& model = transform.GetMatrix();
						const glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(model)));
                        const uint32_t materialIndex = materialIndices.at(material.handle.GetId());

						triangles.reserve(triangles.size() + indices.size() / 3);


                        for (size_t i = 0; i < indices.size(); i += 3)
                        {
                            Triangle triangle;
                            for (size_t j = 0; j < 3; j++)
                            {
                                const Graphics::Vertex& vertex = vertices[indices[i + j]];

                                const glm::vec3 worldPosition = glm::vec3(model * glm::vec4(vertex.position, 1.0f));
                                const glm::vec3 worldNormal = glm::normalize(normal * vertex.normal);

                                glm::vec3 worldTangent = glm::mat3(model) * glm::vec3(vertex.tangent);
                                worldTangent = glm::normalize(worldTangent);
                                worldTangent = glm::normalize(worldTangent - worldNormal * glm::dot(worldNormal, worldTangent));

                                triangle.vertices[j].position = Utils::Glm::ToFloat3(worldPosition);
                                triangle.vertices[j].normal = Utils::Glm::ToFloat3(worldNormal);
                                triangle.vertices[j].tangent = Utils::Glm::ToFloat4(glm::vec4(worldTangent, vertex.tangent.w));
                                triangle.vertices[j].uv = Utils::Glm::ToFloat2(vertex.uv);
							}
							triangle.materialIndex = materialIndex;
                            triangles.push_back(triangle);
                        }
                    });
            return triangles;
        }

        std::expected<DeviceBvh, Core::Utils::Error> BuildBvh(std::vector<Triangle> triangles)
        {
			HostBvh bvh(std::move(triangles));
			DeviceBvh deviceBvh;
			CORE_TRY_VOID(deviceBvh.Build(bvh.GetRoot(), bvh.GetDepth(), bvh.GetNodeCount(), bvh.GetTriangles()));
            return deviceBvh;
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
        StopRendering();
    }

    std::expected<void, Core::Utils::Error> Renderer::InitializeRenderingBuffers(uint32_t width, uint32_t height)
    {
		CORE_TRY_VOID(m_PathPool.Allocate(PathPoolSize));
        for (auto& rayQueue : m_RayQueues)
        {
            CORE_TRY_VOID(rayQueue.Allocate(PathPoolSize, sizeof(uint32_t)));
        }

		CORE_TRY_VOID(m_HitQueue.Allocate(PathPoolSize, sizeof(HitData)));
		CORE_TRY_VOID(m_RegenQueue.Allocate(PathPoolSize, sizeof(uint32_t)));
		CORE_TRY_VOID(m_AccumulationBuffer.Allocate(width * height, sizeof(float4)));
        CORE_TRY_VOID(m_Framebuffer.Allocate(width * height, sizeof(uchar4)));
        m_Width = width;
        m_Height = height;
        CORE_TRY_VOID(m_Framebuffer.GetDeviceBuffer().MemsetBytesSync(0));
		CORE_TRY_VOID(m_Framebuffer.CopyDeviceToHostSync());
		return {};
    }

    std::expected<void, Core::Utils::Error> Renderer::InitializeSceneBuffers(const entt::registry& sceneRegistry, const Assets::Storage& storage)
    {
        MaterialTable materialTable = BuildMaterialTable(sceneRegistry, storage);
		CORE_TRY(materialBuffer, BuildMaterialBuffer(materialTable.materials));
		m_MaterialBuffer = std::move(materialBuffer);

		std::vector<Triangle> triangleList = BuildTriangleList(sceneRegistry, storage, materialTable.indices);
		CORE_TRY(bvh, BuildBvh(std::move(triangleList)));
		m_Bvh = std::move(bvh);
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
		uint32_t pathDepthLimit)
    {
        width = std::min(std::max(width, PathTracerDefaults::MinFrameWidth), PathTracerDefaults::MaxFrameWidth);
        height = std::min(std::max(height, PathTracerDefaults::MinFrameHeight), PathTracerDefaults::MaxFrameHeight);

        samplesPerPixel = std::min(std::max(samplesPerPixel, PathTracerDefaults::MinSamplesPerPixel), PathTracerDefaults::MaxSamplesPerPixel);
        pathDepthLimit = std::min(std::max(pathDepthLimit, PathTracerDefaults::MinPathDepthLimit), PathTracerDefaults::MaxPathDepthLimit);

        if (m_IsRendering.load(std::memory_order_relaxed))
            return std::unexpected(Core::Utils::Error("Renderer is already rendering"));

        if (m_RenderThread.joinable())
            m_RenderThread.join();

        {
            std::scoped_lock lock(m_ErrorMutex);
            m_LastError.reset();
        }

        if (width != m_Width || height != m_Height)
        {
            CORE_TRY_VOID(m_Framebuffer.Allocate(width * height, sizeof(uchar4)));
            CORE_TRY_VOID(m_AccumulationBuffer.Allocate(width * height, sizeof(float4)));
            m_Width = width;
            m_Height = height;
		}

		m_Camera = camera;
		m_CameraMotionStates = std::move(cameraMotionStates);
		m_SampleGridSize = static_cast<uint32_t>(std::sqrt(samplesPerPixel));
		m_SamplesPerPixel = m_SampleGridSize * m_SampleGridSize;
		m_PathDepthLimit = pathDepthLimit;

        m_DoneFrames.store(0, std::memory_order_relaxed);
		m_TotalFrames = static_cast<uint32_t>(m_CameraMotionStates.size());
		m_DoneSamples.store(0, std::memory_order_relaxed);
		m_TotalSamples = static_cast<uint64_t>(m_SamplesPerPixel) * static_cast<uint64_t>(width) * static_cast<uint64_t>(height);
        
        m_IsRendering.store(true, std::memory_order_relaxed);
        // SimulationLoop(std::stop_token{}, startFrame);

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

    void Renderer::StopRendering()
    {
        if (m_RenderThread.joinable())
        {
            m_RenderThread.request_stop();
            m_RenderThread.join();
        }

        m_IsRendering = false;
    }

    void Renderer::SimulationLoop(std::stop_token stopToken, uint32_t startFrame)
    {
        float aspect = static_cast<float>(m_Width) / static_cast<float>(m_Height);
        uint32_t generateCount = std::min(static_cast<uint32_t>(m_TotalSamples), static_cast<uint32_t>(m_PathPool.GetPathCount()));

        for (uint32_t frameIndex = startFrame; frameIndex < m_TotalFrames; frameIndex++)
        {
            DeviceCamera camera = MakeDeviceCamera(m_Camera, aspect, m_CameraMotionStates[frameIndex]);
            auto result = RenderFrame(stopToken, camera, generateCount);
            if (!result)
            {
                std::scoped_lock lock(m_ErrorMutex);
                m_LastError = result.error();
                m_DoneSamples.store(0, std::memory_order_relaxed);
                break;
            }
            if (stopToken.stop_requested()) break;
			m_DoneFrames.fetch_add(1, std::memory_order_relaxed);
        }

        m_IsRendering.store(false, std::memory_order_relaxed);
    }

    std::expected<void, Core::Utils::Error> Renderer::RenderFrame(
        std::stop_token stopToken, 
        DeviceCamera camera,
        uint32_t generateCount)
    {
        m_DoneSamples.store(0, std::memory_order_relaxed);

        uint32_t currentQueue = 0;
        CORE_TRY_VOID(m_RayQueues[currentQueue].ResetCounter());
        CORE_TRY_VOID(m_AccumulationBuffer.MemsetBytesSync(0));
        
        CORE_CUDA_TRY_KERNEL("InitializePaths", 
            Kernels::InitializePaths(
                camera, 
                m_Width,
                m_Height,
                m_SampleGridSize,
                generateCount,
                m_PathPool.GetView(),
                m_RayQueues[currentQueue].GetView<uint32_t>()));
        if (stopToken.stop_requested()) return {};
        m_RayQueues[currentQueue].SetCounterHostValue(generateCount);
        CORE_TRY_VOID(m_RayQueues[currentQueue].SyncCounterFromHost());

        uint64_t launchedSampleCount = generateCount;
        uint32_t iteration = 0;

        while (m_RayQueues[currentQueue].GetCounterHostValue() > 0)
        {
            CORE_TRY_VOID(m_HitQueue.ResetCounter());
            CORE_TRY_VOID(m_RegenQueue.ResetCounter());
            CORE_CUDA_TRY_KERNEL("IntersectRaysWithScene", 
                Kernels::IntersectRaysWithScene(
                    m_RayQueues[currentQueue].GetCounterHostValue(),
                    m_PathPool.GetView(),
                    m_RayQueues[currentQueue].GetView<uint32_t>(),
                    m_Bvh.GetView(),
                    m_HitQueue.GetView<HitData>(),
                    m_RegenQueue.GetView<uint32_t>()));
            if (stopToken.stop_requested()) return {};
            CORE_TRY_VOID(m_HitQueue.SyncCounterFromDevice());
            uint32_t nextQueue = currentQueue ^ 1;
            CORE_TRY_VOID(m_RayQueues[nextQueue].ResetCounter());
            CORE_CUDA_TRY_KERNEL("ResolveHits", 
                Kernels::ResolveHits(
                    m_HitQueue.GetCounterHostValue(),
                    m_PathPool.GetView(),
                    m_HitQueue.GetView<HitData>(),
                    m_Bvh.GetView().triangles,
                    m_MaterialBuffer.GetView<Material>(),
                    m_PathDepthLimit,
                    m_RegenQueue.GetView<uint32_t>(),
                    m_AccumulationBuffer.GetView<float4>(),
                    m_RayQueues[nextQueue].GetView<uint32_t>()));
            if (stopToken.stop_requested()) return {};
            CORE_TRY_VOID(m_RegenQueue.SyncCounterFromDevice());
            uint32_t regenerateCount = std::min<uint64_t>(m_RegenQueue.GetCounterHostValue(), m_TotalSamples - launchedSampleCount);
            CORE_CUDA_TRY_KERNEL("RegeneratePaths", 
                Kernels::RegeneratePaths(
                    regenerateCount,
                    m_RegenQueue.GetView<uint32_t>(),
                    launchedSampleCount,
                    camera,
                    m_Width,
                    m_Height,
                    m_SampleGridSize,
                    m_PathPool.GetView(),
                    m_RayQueues[nextQueue].GetView<uint32_t>()));
            if (stopToken.stop_requested()) return {};
            uint32_t nextRayCount = m_RayQueues[currentQueue].GetCounterHostValue() - m_RegenQueue.GetCounterHostValue() + regenerateCount;
            m_RayQueues[nextQueue].SetCounterHostValue(nextRayCount);
            CORE_TRY_VOID(m_RayQueues[nextQueue].SyncCounterFromHost());

            currentQueue = nextQueue;
            launchedSampleCount += regenerateCount;
            m_DoneSamples.fetch_add(regenerateCount, std::memory_order_relaxed);
            // spdlog::info("Completed iteration {}, hitCount: {}, regenerateCount: {}, nextRayCount: {}, total samples completed: {}/{}", iteration, m_HitQueue.GetCounterHostValue(), regenerateCount, nextRayCount, launchedSampleCount, m_TotalSamples);
            iteration++;
        }
        m_DoneSamples.store(m_TotalSamples, std::memory_order_relaxed);
        
        CORE_CUDA_TRY_KERNEL("PostprocessAccumulatedRadiance", 
            Kernels::PostprocessAccumulatedRadiance(
                m_AccumulationBuffer.GetView<float4>(),
                1.0f / static_cast<float>(m_SamplesPerPixel),
                m_Framebuffer.GetDeviceView<uchar4>()));

        std::scoped_lock lock(m_FrameMutex);
        return m_Framebuffer.CopyDeviceToHostSync();
    }

    std::expected<void, Core::Utils::Error> Renderer::RenderClear(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        const uchar4 clearColor = make_uchar4(r, g, b, a);

        CORE_CUDA_TRY_KERNEL("Clear", Kernels::Clear(clearColor, m_Framebuffer.GetDeviceView<uchar4>()));

        std::scoped_lock lock(m_FrameMutex);
        return m_Framebuffer.CopyDeviceToHostSync();
    }
}
