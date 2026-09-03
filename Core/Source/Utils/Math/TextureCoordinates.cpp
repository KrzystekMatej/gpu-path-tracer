#include <Core/Utils/Math/TextureCoordinates.hpp>

namespace Core::Utils::Math
{
    uint32_t SmallestExtentAxis(const glm::vec3& extent)
    {
        if (extent.x <= extent.y && extent.x <= extent.z)
            return 0;

        if (extent.y <= extent.x && extent.y <= extent.z)
            return 1;

        return 2;
    }

    std::pair<uint32_t, uint32_t> ProjectionAxes(const glm::vec3& extent)
    {
        uint32_t droppedAxis = SmallestExtentAxis(extent);

        switch (droppedAxis)
        {
        case 0:
            return { 1, 2 }; // YZ
        case 1:
            return { 0, 2 }; // XZ
        case 2:
            return { 0, 1 }; // XY
        }

        return { 0, 1 };
    }

    float SafeNormalize(float value, float minimum, float extent)
    {
        constexpr float minimumExtent = 1e-8f;

        if (extent <= minimumExtent)
            return 0.0f;

        return (value - minimum) / extent;
    }

    void GeneratePlanarUVs(std::vector<Graphics::Vertex>& vertices)
    {
        if (vertices.empty())
            return;

        glm::vec3 minimum(std::numeric_limits<float>::max());
        glm::vec3 maximum(std::numeric_limits<float>::lowest());

        for (const Graphics::Vertex& vertex : vertices)
        {
            minimum = glm::min(minimum, vertex.position);
            maximum = glm::max(maximum, vertex.position);
        }

        const glm::vec3 extent = maximum - minimum;
        const std::pair<uint32_t, uint32_t> axes = ProjectionAxes(extent);

        const uint32_t uAxis = axes.first;
        const uint32_t vAxis = axes.second;

        for (Graphics::Vertex& vertex : vertices)
        {
            vertex.uv.x = SafeNormalize(vertex.position[uAxis], minimum[uAxis], extent[uAxis]);
            vertex.uv.y = SafeNormalize(vertex.position[vAxis], minimum[vAxis], extent[vAxis]);
        }
    }
}