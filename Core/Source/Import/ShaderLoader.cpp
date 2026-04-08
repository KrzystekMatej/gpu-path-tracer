#include <Core/Import/ShaderLoader.hpp>
#include <fstream>


namespace Core::Import
{
	std::expected<Shader, Utils::Error> LoadShader(const std::filesystem::path& path, Graphics::Gl::ShaderType type)
	{
		std::ifstream file(path);
		if (!file.is_open())
			return std::unexpected(Utils::Error("Failed to open shader file '{}'", path.string()));

		std::stringstream buffer;
		buffer << file.rdbuf();

		return Shader
		{
			.source = buffer.str(),
			.type = type
		};
	}
}