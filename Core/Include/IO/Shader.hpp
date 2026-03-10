#pragma once
#include <string>
#include "Graphics/Gl/ShaderType.hpp"

namespace Core::IO
{
	struct Shader
	{
		std::string source;
		Graphics::Gl::ShaderType type;
	};
}