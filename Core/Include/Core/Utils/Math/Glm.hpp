#pragma once
#include <string>
#include <format>
#include <Core/External/Glm.hpp>

namespace Core::Utils::Math
{
    inline std::string ToString(const glm::vec2& v)
    {
        return std::format("({}, {})", v.x, v.y);
    }

    inline std::string ToString(const glm::vec3& v)
    {
        return std::format("({}, {}, {})", v.x, v.y, v.z);
    }

    inline std::string ToString(const glm::vec4& v)
    {
        return std::format("({}, {}, {}, {})", v.x, v.y, v.z, v.w);
    }

    inline std::string ToString(const glm::quat& q)
    {
        return std::format("({}, {}, {}, {})", q.w, q.x, q.y, q.z);
    }
    
    inline std::string ToStringEulerRadians(const glm::quat& q)
    {
        glm::vec3 euler = glm::eulerAngles(q);
        return std::format("({}, {}, {})", euler.x, euler.y, euler.z);
    }

    inline std::string ToStringEulerDegrees(const glm::quat& q)
    {
        glm::vec3 euler = glm::degrees(glm::eulerAngles(q));
        return std::format("({}, {}, {})", euler.x, euler.y, euler.z);
    }
}