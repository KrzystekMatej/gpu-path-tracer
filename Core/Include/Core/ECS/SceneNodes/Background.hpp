#pragma once
#include <Core/ECS/SceneNodes/Builder.hpp>

namespace Core::ECS::SceneNodes
{
	class BackgroundBuilder : public Builder
	{
	public:
		std::expected<void, Utils::Error> Build(
			const BuildContext& context,
			entt::registry& registry,
			Assets::Manager& assetManager) const override;
	};
}