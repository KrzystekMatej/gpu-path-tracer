#pragma once
#include "Assets/Types.hpp"

namespace Core::ECS::Components
{
    struct Mesh
    {
        explicit Mesh(Assets::Handle<Assets::Mesh> handle) : handle(handle) {}

		Assets::Handle<Assets::Mesh> handle;
    };

    struct Material
    {
        explicit Material(Assets::Handle<Assets::Material> handle) : handle(handle) {}

		Assets::Handle<Assets::Material> handle;
    };


    struct Background
    {
		explicit Background(Assets::Handle<Assets::EnvironmentMap> handle) : handle(handle) {}

		Assets::Handle<Assets::EnvironmentMap> handle;
    };

    struct Light
    {
        Light() = default;
		Light(const glm::vec3& color, float intensity)
            : color(color), intensity(intensity) {}

        glm::vec3 color{ 1.0f, 1.0f, 1.0f };
        float intensity = 1.0f;
	};
}