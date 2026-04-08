#pragma once
#include <filesystem>
#include <expected>
#include <Core/Utils/Error.hpp>
#include <Core/Import/Shader.hpp>

namespace Core::Import
{
	std::expected<Shader, Utils::Error> LoadShader(const std::filesystem::path& path, Graphics::Gl::ShaderType type);
}