#include <Core/Utils/Math/Normal.hpp>

namespace Core::Utils::Math
{
    void GenerateGeometricNormals(std::vector<Graphics::Vertex>& vertices, const std::vector<uint32_t>& indices)
    {
        for (auto& vertex : vertices)
        {
            vertex.normal = glm::vec3(0.0f);
        }

        for (size_t i = 0; i + 2 < indices.size(); i += 3)
        {
            uint32_t index0 = indices[i];
            uint32_t index1 = indices[i + 1];
            uint32_t index2 = indices[i + 2];

            const glm::vec3& v0 = vertices[index0].position;
            const glm::vec3& v1 = vertices[index1].position;
            const glm::vec3& v2 = vertices[index2].position;

            glm::vec3 edge1 = v1 - v0;
            glm::vec3 edge2 = v2 - v0;
            glm::vec3 faceNormal = glm::cross(edge1, edge2);
            float length = glm::length(faceNormal);

            if (length <= 1e-12f)
                continue;

            faceNormal /= length;

            vertices[index0].normal += faceNormal;
            vertices[index1].normal += faceNormal;
            vertices[index2].normal += faceNormal;
        }

        for (auto& vertex : vertices)
        {
            const float length = glm::length(vertex.normal);

            if (length > 1e-12f)
                vertex.normal /= length;
            else
                vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }
    }
}