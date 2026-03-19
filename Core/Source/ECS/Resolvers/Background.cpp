#include "ECS/Resolvers/Background.hpp"
#include "Utils/Yaml.hpp"
#include "ECS/Components/Graphics.hpp"

namespace Core::ECS
{
	std::expected<void, Utils::Error> BackgroundResolver::Resolve(
		const SceneNodeContext& context,
		entt::registry& registry,
		Assets::Manager& assetManager) const
	{
		auto pathResult = Utils::Yaml::GetString(context.node, "path");
		if (!pathResult)
			return std::unexpected(Utils::Error(std::move(pathResult).error()));
		auto envMapHandleResult = assetManager.ImportEnvironmentMap(std::move(pathResult).value(), Graphics::ColorSpace::Linear);
		if (!envMapHandleResult)
			return std::unexpected(Utils::Error(std::move(envMapHandleResult).error()));
		registry.emplace<Components::Background>(context.entity, envMapHandleResult.value());
		return {};
	}
}