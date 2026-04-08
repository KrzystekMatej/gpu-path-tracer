#pragma once
#include <entt/entt.hpp>
#include <Core/External/Glm.hpp>
#include <Core/Utils/Math/CoordinateSystem.hpp>

namespace Core::ECS
{
    class Scene;

    namespace Systems
    {
		void PropagateTransform(entt::registry& registry, entt::entity entity, const glm::mat4& parentTransform);
    }
}

namespace Core::ECS::Components
{

    struct WorldTransform
    {
		WorldTransform() = default;
		explicit WorldTransform(const glm::mat4& matrix) : matrix(matrix) {}

		const glm::mat4& GetMatrix() const { return matrix; }
    private:
		friend void Systems::PropagateTransform(entt::registry&, entt::entity, const glm::mat4&);

        glm::mat4 matrix{ 1.0f };
    };

    struct Transform
    {
        glm::vec3 translation{ 0.0f, 0.0f, 0.0f };
        glm::quat rotation = glm::identity<glm::quat>();
        glm::vec3 scale{ 1.0f, 1.0f, 1.0f };

        Transform() = default;
        Transform(const glm::vec3& translation, const glm::quat& rotation, const glm::vec3& scale)
            : translation(translation), rotation(rotation), scale(scale) {}

        glm::mat4 GetMatrix() const
        {
            glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);
            glm::mat4 rotationMatrix = glm::toMat4(rotation);
            glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), scale);
            return translationMatrix * rotationMatrix * scaleMatrix;
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
