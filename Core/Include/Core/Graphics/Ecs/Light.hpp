#pragma once
#include <Core/Ecs/Builder.hpp>
#include <Core/External/Glm.hpp>

namespace Core::Graphics::Ecs
{
    struct Light
    {
        Light() = default;
        Light(const glm::vec3& color, float intensity)
            : color(color), intensity(intensity) {
        }

        glm::vec3 color{ 1.0f, 1.0f, 1.0f };
        float intensity = 1.0f;
    };

	class LightBuilder : public Core::Ecs::Builder
	{
	public:
		std::expected<void, Utils::Error> Build(
			const Core::Ecs::BuildContext& context,
			entt::registry& registry,
			Assets::Manager& assetManager) const override;
	};
}
