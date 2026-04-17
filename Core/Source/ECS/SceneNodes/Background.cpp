#include <Core/Ecs/SceneNodes/Background.hpp>
#include <Core/Utils/Yaml.hpp>
#include <Core/Ecs/Components/Graphics.hpp>

namespace Core::Ecs::SceneNodes
{
	std::expected<void, Utils::Error> BackgroundBuilder::Build(
		const BuildContext& context,
		entt::registry& registry,
		Assets::Manager& assetManager) const
	{
		auto pathResult = Utils::Yaml::GetString(context.node, "path");
		if (!pathResult)
			return std::unexpected(Utils::Error(std::move(pathResult).error()));
		auto envMapHandleResult = assetManager.ImportEnvironmentMap(std::move(pathResult).value(), Graphics::Common::ColorSpace::Linear);
		if (!envMapHandleResult)
			return std::unexpected(Utils::Error(std::move(envMapHandleResult).error()));
		registry.emplace<Components::Background>(context.entity, envMapHandleResult.value());
		return {};
	}
}