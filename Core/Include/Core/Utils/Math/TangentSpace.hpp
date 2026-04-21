#pragma once
#include <vector>
#include <Core/Graphics/Common/Vertex.hpp>

namespace Core::Utils::Math
{
	void GenerateTangentSpace(std::vector<Graphics::Vertex>& vertices, const std::vector<uint32_t>& indices);
}
