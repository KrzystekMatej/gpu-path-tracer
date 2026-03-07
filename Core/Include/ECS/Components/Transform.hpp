#pragma once
#include <entt/entt.hpp>
#include "External/Glm.hpp"

namespace Core::ECS::Components
{

    struct WorldTransform
    {
        glm::mat4 matrix{ 1.0f };
    };

    struct Transform
    {
        static constexpr glm::vec3 WorldUp{ 0.0f, 1.0f, 0.0f };

        glm::vec3 position{ 0.0f, 0.0f, 0.0f };
        glm::quat rotation = glm::identity<glm::quat>();
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };
        entt::entity parent{ entt::null };

        Transform() = default;
        Transform(const glm::vec3& position, const glm::quat& rotation, const glm::vec3& scale, entt::entity parent)
            : position(position), rotation(rotation), scale(scale), parent(parent) {
        }

        glm::mat4 GetMatrix() const
        {
            glm::mat4 translation = glm::translate(glm::mat4(1.0f), position);
            glm::mat4 rotationMat = glm::toMat4(rotation);
            glm::mat4 scaleMat = glm::scale(glm::mat4(1.0f), scale);
            return translation * rotationMat * scaleMat;
        }

        glm::vec3 GetForward() const
        {
            return rotation * glm::vec3(0.0f, 0.0f, -1.0f);
        }

        glm::vec3 GetRight() const
        {
            return rotation * glm::vec3(1.0f, 0.0f, 0.0f);
        }
        glm::vec3 GetUp() const
        {
            return rotation * glm::vec3(0.0f, 1.0f, 0.0f);
        }
    };
}
