#pragma once
#include <Core/External/Glm.hpp>

namespace Core::Utils::Math
{
    glm::vec3 ExtractTranslation(const glm::mat4& transform)
    {
        return glm::vec3(transform[3]);
    }

    glm::quat ExtractRotation(const glm::mat4& transform)
    {
        glm::vec3 x = glm::vec3(transform[0]);
        glm::vec3 y = glm::vec3(transform[1]);
        glm::vec3 z = glm::vec3(transform[2]);

        x = glm::normalize(x);
        y = glm::normalize(y);
        z = glm::normalize(z);

        glm::mat3 rotation(x, y, z);
        return glm::normalize(glm::quat_cast(rotation));
    }
}