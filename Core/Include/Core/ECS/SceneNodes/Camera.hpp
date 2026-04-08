#pragma once
#include <Core/ECS/SceneNodes/Builder.hpp>

namespace Core::ECS::SceneNodes
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
