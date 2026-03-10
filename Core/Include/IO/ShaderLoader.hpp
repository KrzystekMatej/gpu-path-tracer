#pragma once
#include <filesystem>
#include <expected>
#include "Utils/Error/Error.hpp"
#include "IO/Shader.hpp"

namespace Core::IO
{
	std::expected<Shader, Utils::Error> LoadShader(const std::filesystem::path& path, Graphics::Gl::ShaderType type);
}