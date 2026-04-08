#pragma once
#include <string>
#include <Core/Graphics/Gl/ShaderType.hpp>

namespace Core::Import
{
	struct Shader
	{
		std::string source;
		Graphics::Gl::ShaderType type;
	};
}