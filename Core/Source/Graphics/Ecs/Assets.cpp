#include <Core/Graphics/Ecs/Assets.hpp>
#include <Core/Ecs/Transform.hpp>
#include <Core/Utils/Yaml.hpp>
#include <Core/Ecs/Hierarchy.hpp>
#include <Core/Graphics/Ecs/Light.hpp>
#include <execution>
#include <numeric>

namespace Core::Graphics::Ecs
{
	namespace 
	{
		glm::vec3 GetMeshCentroid(const Cpu::Mesh& mesh)
		{
			const std::vector<Vertex>& vertices = mesh.GetVertices();
			if (vertices.empty())
				return glm::vec3(0.0f);

			const glm::vec3 centroid = std::transform_reduce(
				vertices.begin(),
				vertices.end(),
				glm::vec3(0.0f),
				std::plus<>(),
				[](const Vertex& vertex)
				{
					return vertex.position;
				});
				
			return centroid / static_cast<float>(vertices.size());
		}
		
		glm::vec3 GetAverageEmission(const Cpu::Texture& emissionTexture)
		{
			std::span<const glm::vec3> pixelView = emissionTexture.GetPixelView<glm::vec3>();
			if (pixelView.empty())
				return glm::vec3(0.0f);
			
			glm::vec3 averageColor = std::reduce(pixelView.begin(), pixelView.end(), glm::vec3(0.0f));
			return averageColor / static_cast<float>(pixelView.size());
		}

		float GetSurfaceArea(const Cpu::Mesh& mesh)
		{
			float area = 0.0f;
			const std::vector<Vertex>& vertices = mesh.GetVertices();
			const std::vector<uint32_t>& indices = mesh.GetIndices();

			for (size_t i = 0; i + 2 < indices.size(); i += 3)
			{
				const glm::vec3& v0 = vertices[indices[i]].position;
				const glm::vec3& v1 = vertices[indices[i + 1]].position;
				const glm::vec3& v2 = vertices[indices[i + 2]].position;

				area += 0.5f * glm::length(glm::cross(v1 - v0, v2 - v0));
			}

			return area;
		}

		float MaxChannel(const glm::vec3& color)
		{
			return std::max({ color.r, color.g, color.b });
		}
	}

	std::expected<void, Utils::Error> ModelBuilder::Build(
		const Core::Ecs::BuildContext& context,
		entt::registry& registry,
		Assets::Manager& assetManager) const
	{
		CORE_TRY(path, Utils::Yaml::GetValue<std::string>(context.node, "path"));
		CORE_TRY(modelHandle, assetManager.ImportObj(path));

		const Assets::Storage& storage = assetManager.GetStorage();

		const Assets::Model& model = storage.Get(modelHandle).value();
		std::vector<entt::entity> children;
		children.reserve(model.parts.size());

		for (const auto& part : model.parts)
		{
			const entt::entity child = registry.create();

			children.emplace_back(child);
			registry.emplace<Core::Ecs::Parent>(child, context.entity);
			registry.emplace<Core::Ecs::Transform>(child);
			registry.emplace<Core::Ecs::WorldTransform>(child);
			registry.emplace<Mesh>(child, part.mesh);
			registry.emplace<Material>(child, part.material);

			if (storage.Get(part.material).value().get().surface == Graphics::SurfaceModel::Emissive)
			{
				entt::entity lightEntity = registry.create();
				registry.emplace<Core::Ecs::Children>(child, std::vector<entt::entity>{ lightEntity });
				registry.emplace<Core::Ecs::Parent>(lightEntity, child);
				glm::vec3 centroid = GetMeshCentroid(storage.Get(part.mesh).value().get().cpu);
				registry.emplace<Core::Ecs::Transform>(lightEntity, GetMeshCentroid(storage.Get(part.mesh).value().get().cpu));
				registry.emplace<Core::Ecs::WorldTransform>(lightEntity);

				const Cpu::Texture& emissionTexture = storage.Get(storage.Get(part.material).value().get().emission).value().get().cpu;
				glm::vec3 rgbIntensity = GetAverageEmission(emissionTexture) * GetSurfaceArea(storage.Get(part.mesh).value().get().cpu);
				float intensity = MaxChannel(rgbIntensity);
				glm::vec3 color = intensity > 0.0f ? rgbIntensity / intensity : glm::vec3(1.0f);
				registry.emplace<Light>(lightEntity, color, intensity);
			}
		}

		if (!children.empty())
			registry.emplace<Core::Ecs::Children>(context.entity, std::move(children));

		return {};
	}

	std::expected<void, Utils::Error> BackgroundBuilder::Build(
		const Core::Ecs::BuildContext& context,
		entt::registry& registry,
		Assets::Manager& assetManager) const
	{
		CORE_TRY(path, Utils::Yaml::GetValue<std::string>(context.node, "path"));
		CORE_TRY(envMapHandle, assetManager.ImportEnvironmentMap(path, Graphics::ColorSpace::Linear));
		registry.emplace<Background>(context.entity, envMapHandle);
		return {};
	}
	
	std::expected<void, Utils::Error> GridBuilder::Build(
		const Core::Ecs::BuildContext& context,
		entt::registry& registry,
		Assets::Manager& assetManager) const
	{
		CORE_TRY(rows, Utils::Yaml::GetValue<uint32_t>(context.node, "rows"));
		CORE_TRY(columns, Utils::Yaml::GetValue<uint32_t>(context.node, "columns"));
		CORE_TRY(spacing, Utils::Yaml::GetValue<float>(context.node, "spacing"));

		registry.emplace<Grid>(context.entity, rows, columns, spacing);
		std::vector<entt::entity> children;
		children.reserve(rows * columns);

		for (uint32_t i = 0; i < rows; i++)
		{
			for (uint32_t j = 0; j < columns; j++)
			{
				entt::entity cellEntity = registry.create();

				children.emplace_back(cellEntity);
				registry.emplace<Core::Ecs::Parent>(cellEntity, context.entity);

				glm::vec3 position = 
				{
					static_cast<float>(static_cast<int32_t>(j) - static_cast<int32_t>(columns) / 2) * spacing,
					static_cast<float>(static_cast<int32_t>(i) - static_cast<int32_t>(rows) / 2) * spacing,
					0.0f
				};

				registry.emplace<Core::Ecs::Transform>(cellEntity, position);
				registry.emplace<Core::Ecs::WorldTransform>(cellEntity);

				Core::Ecs::BuildContext cellContext{ .node = context.node, .entity = cellEntity, .parent = context.entity };
				CORE_TRY_DISCARD(ModelBuilder().Build(cellContext, registry, assetManager));
			}
		}

		if (!children.empty())
			registry.emplace<Core::Ecs::Children>(context.entity, std::move(children));
		return {};
	}
}