#pragma once
#include <Core/External/Glm.hpp>

namespace Core::Utils::Math
{
    glm::vec3 ExtractTranslation(const glm::mat4& transform)
    {
        return glm::vec3(transform[3]);
    }

    glm::quat ExtractRotationPure(const glm::mat4& transform)
    {
        return glm::normalize(glm::quat_cast(glm::mat3(transform)));
    }

    glm::quat ExtractRotationUniformScale(const glm::mat4& transform)
    {
        glm::mat3 rotation(transform);

        const float scale = glm::length(rotation[0]);
        constexpr float minimumScale = 1e-6f;

        assert(scale > minimumScale);

        if (scale <= minimumScale)
        {
            return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        }

        rotation /= scale;

        return glm::normalize(glm::quat_cast(rotation));
    }
    
    glm::vec3 ExtractScale(const glm::mat4& transform)
    {
        glm::vec3 scale;
        scale.x = glm::length(glm::vec3(transform[0]));
        scale.y = glm::length(glm::vec3(transform[1]));
        scale.z = glm::length(glm::vec3(transform[2]));
        return scale;
    }
}