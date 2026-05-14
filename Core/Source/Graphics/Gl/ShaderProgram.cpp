#include <glad/gl.h>
#include <Core/Graphics/Gl/ShaderProgram.hpp>
#include <utility>

namespace Core::Graphics::Gl
{
	ShaderProgram::ShaderProgram()
	{
		m_Id = glCreateProgram();
	}

	ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
		: m_Id(std::exchange(other.m_Id, 0)),
		m_UniformLocationCache(std::move(other.m_UniformLocationCache)),
		m_TextureUnitCounter(other.m_TextureUnitCounter) { }

	ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept
	{
		if (this != &other)
		{
			if (m_Id)
				glDeleteProgram(m_Id);

			m_Id = std::exchange(other.m_Id, 0);
			m_UniformLocationCache = std::move(other.m_UniformLocationCache);
			m_TextureUnitCounter = other.m_TextureUnitCounter;
		}
		return *this;
	}

	ShaderProgram::~ShaderProgram()
	{
		if (m_Id)
		{
			glDeleteProgram(m_Id);
			m_Id = 0;
		}
	}

	std::expected<ShaderProgram, Utils::Error> ShaderProgram::Create(std::span<const Shader> shaders)
	{
		ShaderProgram program;
		for (const auto& shader : shaders)
		{
			program.AttachShader(shader);
		}

		CORE_TRY_DISCARD(program.Link());

		for (const auto& shader : shaders)
		{
			program.DetachShader(shader);
		}
		return program;
	}

	std::expected<void, Utils::Error> ShaderProgram::Link()
	{
		glLinkProgram(m_Id);
		int linkStatus;
		glGetProgramiv(m_Id, GL_LINK_STATUS, &linkStatus);
		if (!linkStatus)
		{
			int logLength;
			glGetProgramiv(m_Id, GL_INFO_LOG_LENGTH, &logLength);
			std::string log(logLength, '\0');
			glGetProgramInfoLog(m_Id, logLength, nullptr, log.data());
			return std::unexpected(Utils::Error("Failed to link program: {}", log));
		}
		return {};
	}

	void ShaderProgram::AttachShader(const Shader& shader)
	{
		glAttachShader(m_Id, shader.m_Id);
	}

	void ShaderProgram::DetachShader(const Shader& shader)
	{
		glDetachShader(m_Id, shader.m_Id);
	}

	void ShaderProgram::Bind() 
	{
		glUseProgram(m_Id);
		m_TextureUnitCounter = 0;
	}

	int ShaderProgram::GetUniformLocation(const std::string& name) 
	{
		auto it = m_UniformLocationCache.find(name);
		if (it != m_UniformLocationCache.end())
		{
			return it->second;
		}

		int location = glGetUniformLocation(m_Id, name.c_str());
		if (location == -1)
		{
			spdlog::error("Uniform '{}' does not exist!", name);
		}

		m_UniformLocationCache[name] = location;
		return location;
	}

	void ShaderProgram::SetInt32(const std::string& name, int value) 
	{
		int location = GetUniformLocation(name);
		if (location != -1) glUniform1i(location, value);
	}

	void ShaderProgram::SetUInt32(const std::string& name, unsigned int value) 
	{
		int location = GetUniformLocation(name);
		if (location != -1) glUniform1ui(location, value);
	}

	void ShaderProgram::SetFloat(const std::string& name, float value) 
	{
		int location = GetUniformLocation(name);
		if (location != -1) glUniform1f(location, value);
	}

	void ShaderProgram::SetVec2(const std::string& name, const glm::vec2& vec) 
	{
		int location = GetUniformLocation(name);
		if (location != -1) glUniform2fv(location, 1, glm::value_ptr(vec));
	}

	void ShaderProgram::SetVec3(const std::string& name, const glm::vec3& vec) 
	{
		int location = GetUniformLocation(name);
		if (location != -1) glUniform3fv(location, 1, glm::value_ptr(vec));
	}

	void ShaderProgram::SetVec4(const std::string& name, const glm::vec4& vec) 
	{
		int location = GetUniformLocation(name);
		if (location != -1) glUniform4fv(location, 1, glm::value_ptr(vec));
	}

	void ShaderProgram::SetMatrix3x3(const std::string& name, const glm::mat3& mat) 
	{
		int location = GetUniformLocation(name);
		if (location != -1) glUniformMatrix3fv(location, 1, GL_FALSE, &mat[0][0]);
	}

	void ShaderProgram::SetMatrix4x4(const std::string& name, const glm::mat4& mat) 
	{
		int location = GetUniformLocation(name);
		if (location != -1) glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
	}

	void ShaderProgram::SetTexture(const std::string& name, const Texture& texture)
	{
		int location = GetUniformLocation(name);
		if (location == -1)
			return;

		glActiveTexture(GL_TEXTURE0 + m_TextureUnitCounter);
		texture.Bind();
		glUniform1i(location, m_TextureUnitCounter);
		m_TextureUnitCounter++;
	}
}