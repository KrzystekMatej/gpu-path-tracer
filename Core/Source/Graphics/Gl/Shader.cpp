#include <glad/gl.h>
#include <Core/Graphics/Gl/Shader.hpp>

namespace Core::Graphics::Gl
{
	namespace 
	{
		std::expected<uint32_t, Utils::Error> GetGlShaderType(ShaderType type)
		{
			switch (type)
			{
				case ShaderType::Vertex:
					return GL_VERTEX_SHADER;
				case ShaderType::Fragment:
					return GL_FRAGMENT_SHADER;
				case ShaderType::Geometry:
					return GL_GEOMETRY_SHADER;
				case ShaderType::Compute:
					return GL_COMPUTE_SHADER;
				case ShaderType::TessellationControl:
					return GL_TESS_CONTROL_SHADER;
				case ShaderType::TessellationEvaluation:
					return GL_TESS_EVALUATION_SHADER;
				default:
					return std::unexpected(Utils::Error("Invalid shader type: {}", static_cast<uint32_t>(type)));
			}
		}
	}


	Shader::Shader(uint32_t type)
	{
		m_Id = glCreateShader(type);
		m_Type = type;
	}

	Shader::Shader(Shader&& other) noexcept
		: m_Id(std::exchange(other.m_Id, 0)), m_Type(std::exchange(other.m_Type, 0)) {}

	Shader& Shader::operator=(Shader&& other) noexcept
	{
		if (this != &other)
		{
			if (m_Id)
				glDeleteShader(m_Id);
			m_Id = std::exchange(other.m_Id, 0);
			m_Type = std::exchange(other.m_Type, 0);
		}
		return *this;
	}

	Shader::~Shader()
	{
		if (m_Id)
		{
			glDeleteShader(m_Id);
			m_Id = 0;
		}
	}

	std::expected<Shader, Utils::Error> Shader::Create(const Import::Shader& shader)
	{
		auto typeResult = GetGlShaderType(shader.type);
		if (!typeResult)
			return std::unexpected(std::move(typeResult).error());
		uint32_t glType = typeResult.value();
		Shader glShader(glType);
		const char* sourceCStr = shader.source.c_str();
		glShaderSource(glShader.m_Id, 1, &sourceCStr, nullptr);
		glCompileShader(glShader.m_Id);
		int compileStatus;
		glGetShaderiv(glShader.m_Id, GL_COMPILE_STATUS, &compileStatus);
		if (!compileStatus)
		{
			int logLength;
			glGetShaderiv(glShader.m_Id, GL_INFO_LOG_LENGTH, &logLength);
			std::string log(logLength, '\0');
			glGetShaderInfoLog(glShader.m_Id, logLength, nullptr, log.data());
			return std::unexpected(Utils::Error("Failed to compile shader: {}", log));
		}
		return glShader;
	}
}