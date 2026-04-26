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

        std::unordered_map<Core::Utils::Guid, std::pair<uint32_t, Material>> BuildMaterialMap(
            const entt::registry& sceneRegistry,
            const Assets::Storage& storage)
        {
            std::unordered_map<Core::Utils::Guid, std::pair<uint32_t, Material>> materials;
            sceneRegistry
                .view<Core::Ecs::WorldTransform, Ecs::Mesh, Ecs::Material>()
                .each([&](const Core::Ecs::WorldTransform& transform, const Ecs::Mesh& mesh, const Ecs::Material& material)
                    {
						const auto& materialAsset = storage.Get(material.handle).value().get();

						if (!materials.count(material.handle.GetId()))
						{
							materials[material.handle.GetId()] = { 
                                materials.size(), {
								ToGlobalShadingUnchecked(materialAsset.surface),
								storage.Get(materialAsset.albedo).value().get().cuda.GetView<float>(),
								storage.Get(materialAsset.roughness).value().get().cuda.GetView<float>(),
								storage.Get(materialAsset.metallic).value().get().cuda.GetView<float>(),
								storage.Get(materialAsset.ao).value().get().cuda.GetView<float>(),
								storage.Get(materialAsset.normal).value().get().cuda.GetView<float>(),
							}};
						}
                    });

            return materials;
        }

        std::expected<Memory::DeviceBuffer1D, Core::Utils::Error> BuildMaterialBuffer(
            const std::unordered_map<Core::Utils::Guid, std::pair<uint32_t, Material>>& materialMap)
        {
            std::vector<Material> materials(materialMap.size());
            for (auto kv : materialMap | std::views::values)
            {
                materials[kv.first] = kv.second;
            }
            Memory::DeviceBuffer1D buffer;
            auto result = buffer.Allocate(materials.size(), sizeof(Material));
            if (!result)
                return std::unexpected(std::move(result).error());
            result = buffer.UploadSync(materials.data(), materials.size());
            if (!result)
                return std::unexpected(std::move(result).error());
            return buffer;
		}

        std::vector<Triangle> BuildTriangleList(
            const entt::registry& sceneRegistry,
            const Assets::Storage& storage,
            const std::unordered_map<Core::Utils::Guid, std::pair<uint32_t, Material>>& materialMap)
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
						glm::mat3 normal = glm::mat3(glm::transpose(glm::inverse(model)));
						triangles.reserve(triangles.size() + indices.size() / 3);

                        for (size_t i = 0; i < indices.size(); i += 3)
                        {
                            Triangle triangle;
                            for (size_t j = 0; j < 3; ++j)
                            {
                                const auto& vertex = vertices[indices[i + j]];
                                triangle.vertices[j].position = Utils::Glm::ToFloat3(model * glm::vec4(vertex.position, 1.0f));
                                glm::vec3 worldNormal = glm::normalize(normal * vertex.normal);
								glm::vec3 tangent = glm::mat3(model) * glm::vec3(vertex.tangent);
								tangent = glm::normalize(tangent);
								tangent = glm::normalize(tangent - worldNormal * glm::dot(worldNormal, tangent));
								triangle.vertices[j].tangent = Utils::Glm::ToFloat4(glm::vec4(tangent, vertex.tangent.w));
                                triangle.vertices[j].uv = Utils::Glm::ToFloat2(vertex.uv);
							}
							triangle.materialIndex = materialMap.at(material.handle.GetId()).first;
                            triangles.push_back(triangle);
                        }
                    });
            return triangles;
        }

        std::expected<DeviceBvh, Core::Utils::Error> BuildBvh(std::vector<Triangle> triangles)
        {
			HostBvh bvh(std::move(triangles));
			DeviceBvh deviceBvh;
			auto result = deviceBvh.Build(bvh.GetRoot(), bvh.GetTriangles());
            if (!result)
				return std::unexpected(std::move(result).error());
            return deviceBvh;
		}
    }

    Renderer::~Renderer()
    {
        StopRendering();
    }

    std::expected<void, Core::Utils::Error> Renderer::InitializeRenderingBuffers(size_t framebufferWidth, size_t framebufferHeight)
    {
		auto result = m_PathPool.Allocate(PathPoolSize);
        if (!result)
			return std::unexpected(std::move(result).error());
        for (auto& rayQueue : m_RayQueues)
        {
            result = rayQueue.Allocate(PathPoolSize, sizeof(Ray));
			if (!result)
                return std::unexpected(std::move(result).error());
		}

		result = m_HitQueue.Allocate(PathPoolSize, sizeof(HitData));
        if (!result)
			return std::unexpected(std::move(result).error());

		result = m_RegenQueue.Allocate(PathPoolSize, sizeof(uint32_t));
		if (!result)
			return std::unexpected(std::move(result).error());


		result = m_AccumulationBuffer.Allocate(framebufferWidth, framebufferHeight, sizeof(float4));
        if (!result)
			return std::unexpected(std::move(result).error());

        result = m_Framebuffer.Allocate(framebufferWidth, framebufferHeight, sizeof(uchar4));
        if (!result)
            return std::unexpected(std::move(result).error());
        result = m_Framebuffer.GetDeviceBuffer().MemsetBytesSync(0);
		if (!result)
            return std::unexpected(std::move(result).error());
		return m_Framebuffer.CopyDeviceToHostSync();
    }

    std::expected<void, Core::Utils::Error> Renderer::InitializeSceneBuffers(const entt::registry& sceneRegistry, const Assets::Storage& storage)
    {
        auto materialMap = BuildMaterialMap(sceneRegistry, storage);
		auto materialBufferResult = BuildMaterialBuffer(materialMap);
        if (!materialBufferResult)
			return std::unexpected(std::move(materialBufferResult).error());
		m_MaterialBuffer = std::move(materialBufferResult).value();

		std::vector<Triangle> triangleList = BuildTriangleList(sceneRegistry, storage, materialMap);
		auto bvhResult = BuildBvh(std::move(triangleList));
		if (!bvhResult)
			return std::unexpected(std::move(bvhResult).error());
		m_Bvh = std::move(bvhResult).value();

        return {};
    }

    std::optional<Core::Utils::Error> Renderer::GetLastError() const
    {
        std::scoped_lock lock(m_ErrorMutex);
        return m_LastError;
    }

    std::expected<void, Core::Utils::Error> Renderer::Clear(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        if (IsRendering())
            return std::unexpected(Core::Utils::Error("Cannot clear renderer while rendering"));

        return RenderClear(r, g, b, a);
    }

    std::expected<void, Core::Utils::Error> Renderer::StartSimulation(
		uint32_t frameWidth,
		uint32_t frameHeight,
		std::vector<Capture::MotionState> cameraMotionStates,
        uint32_t startFrame,
		uint32_t samplesPerPixel,
		uint32_t pathDepth,
        std::chrono::milliseconds frameDelay)
    {
        if (m_IsRendering.exchange(true))
            return std::unexpected(Core::Utils::Error("Renderer is already rendering"));

        if (m_RenderThread.joinable())
            m_RenderThread.join();

        {
            std::scoped_lock lock(m_ErrorMutex);
            m_LastError.reset();
        }

        if (startFrame >= cameraMotionStates.size())
		{
            m_IsRendering.store(false, std::memory_order_relaxed);
            return std::unexpected(Core::Utils::Error("Start frame is out of range - nothing to render"));
        }

        if (frameWidth != m_Framebuffer.GetWidth() || frameHeight != m_Framebuffer.GetHeight())
        {
            auto result = m_Framebuffer.Allocate(frameWidth, frameHeight, sizeof(uchar4));
            if (!result)
            {
				m_IsRendering.store(false, std::memory_order_relaxed);
                return std::unexpected(std::move(result).error());
            }
		}

		m_CameraMotionStates = std::move(cameraMotionStates);
		m_SampleGridSize = static_cast<uint32_t>(std::sqrt(samplesPerPixel));
		m_SamplesPerPixel = m_SampleGridSize * m_SampleGridSize;
		m_PathDepth = pathDepth;

        m_DoneFrames.store(startFrame, std::memory_order_relaxed);
		m_TotalFrames = static_cast<uint32_t>(m_CameraMotionStates.size());
		m_DoneSamples.store(0, std::memory_order_relaxed);
		m_TotalSamples = m_SamplesPerPixel * frameWidth * frameHeight;

        try
        {
            m_RenderThread = std::jthread([this, startFrame, frameDelay](std::stop_token stopToken)
			{
				SimulationLoop(stopToken, startFrame, frameDelay);
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

    void Renderer::SimulationLoop(std::stop_token stopToken, uint32_t startFrame, std::chrono::milliseconds frameDelay)
    {
        for (uint32_t frameIndex = startFrame; frameIndex < m_TotalFrames; ++frameIndex)
        {
            if (stopToken.stop_requested())
                break;

            {
                std::unique_lock lock(m_StopMutex);
                m_StopCv.wait_for(lock, stopToken, frameDelay, [] { return false; });
            }

            if (stopToken.stop_requested())
                break;

            const uchar4 clearColor = MakeSimulationColor(frameIndex);
            auto renderResult = RenderClear(clearColor.x, clearColor.y, clearColor.z, clearColor.w);

            if (!renderResult)
            {
                std::scoped_lock lock(m_ErrorMutex);
                m_LastError = renderResult.error();
                break;
            }

			m_DoneFrames.fetch_add(1, std::memory_order_seq_cst);
        }

        m_IsRendering.store(false, std::memory_order_relaxed);
    }

    std::expected<void, Core::Utils::Error> Renderer::RenderClear(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        const uchar4 clearColor = make_uchar4(r, g, b, a);

        Kernels::Clear(clearColor, m_Framebuffer.GetDeviceBuffer().GetView<uchar4>());

        std::scoped_lock lock(m_FrameMutex);
        return m_Framebuffer.CopyDeviceToHostSync();
    }
}
