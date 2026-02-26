#pragma once
#include "Assets/IO/Types.hpp"
#include "Graphics/Cpu/Resources.hpp"

namespace Core::Assets::IO
{
	Graphics::Cpu::Mesh BuildMesh(ParsedMesh parsedMesh);
}
