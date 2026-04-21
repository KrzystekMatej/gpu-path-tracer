#pragma once
#include <Core/Ecs/Builder.hpp>
#include <Core/External/Glm.hpp>

namespace Core::Graphics::Ecs
{
    struct Camera
    {
    public:
		Camera() = default;
        Camera(float fovY, float nearPlane, float farPlane) : fovY(fovY), nearPlane(nearPlane), farPlane(farPlane) {}

        glm::mat4 GetProjectionMatrix(float aspectRatio) const
        {
            return glm::perspective(fovY, aspectRatio, nearPlane, farPlane);
        }

        float fovY;
        float nearPlane;
        float farPlane;
    };

	class CameraBuilder : public Core::Ecs::Builder
	{
	public:
		virtual std::expected<void, Utils::Error> Build(
			const Core::Ecs::BuildContext& context,
			entt::registry& registry,
			Assets::Manager& assetManager) const override;
	};
}