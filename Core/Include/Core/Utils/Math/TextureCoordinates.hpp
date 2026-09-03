#pragma once
#include <vector>
#include <Core/Graphics/Common/Vertex.hpp>

namespace Core::Utils::Math
{
    void GeneratePlanarUVs(std::vector<Graphics::Vertex>& vertices);
}