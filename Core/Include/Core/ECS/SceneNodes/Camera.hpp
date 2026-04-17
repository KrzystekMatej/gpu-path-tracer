#pragma once
#include <Core/Ecs/SceneNodes/Builder.hpp>

namespace Core::Ecs::SceneNodes
{
	class CameraBuilder : public Builder
	{
	public:
		virtual std::expected<void, Utils::Error> Build(
			const BuildContext& context,
			entt::registry& registry,
			Assets::Manager& assetManager) const override;
	};
}
