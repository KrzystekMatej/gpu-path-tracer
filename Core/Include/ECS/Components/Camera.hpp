#pragma once
#include "External/Glm.hpp"

namespace Core::ECS::Components
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
}