#pragma once
#include <entt/entt.hpp>
#include "External/Glm.hpp"
#include "Utils/Math/CoordinateSystem.hpp"

namespace Core::ECS::Components
{

    struct WorldTransform
    {
		WorldTransform() = default;
		explicit WorldTransform(const glm::mat4& matrix) : matrix(matrix) {}

        glm::mat4 matrix{ 1.0f };
    };

    struct Transform
    {
        glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        glm::quat rotation = glm::identity<glm::quat>();
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

        Transform() = default;
        Transform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale)
            : position(position), rotation(rotation), scale(scale) {}

        glm::mat4 GetMatrix() const
        {
            glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
            glm::mat4 rotationMat = glm::toMat4(rotation);
            glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);
            return translation * rotationMat * scaleMat;
        }

        glm::vec3 GetForward() const
        {
            return rotation * Utils::Math::CoordinateSystem::Forward;
        }

        glm::vec3 GetRight() const
        {
            return rotation * Utils::Math::CoordinateSystem::Right;
        }

        glm::vec3 GetUp() const
        {
            return rotation * Utils::Math::CoordinateSystem::Up;
        }
    };
}
