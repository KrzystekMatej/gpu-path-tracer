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
		m_UniformLocationCache(std::move(other.m_UniformLocationCache)) { }

	ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept
	{
		if (this != &other)
		{
			if (m_Id)
				glDeleteProgram(m_Id);

			m_Id = std::exchange(other.m_Id, 0);
			m_UniformLocationCache = std::move(other.m_UniformLocationCache);
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

	std::expected<ShaderProgram, Utils::Error> ShaderProgram::Create(std::span<Shader> shaders)
	{
		ShaderProgram program;
		for (const auto& shader : shaders)
		{
			program.AttachShader(shader);
		}

		auto ok = program.Link();
		if (!ok)
			return std::unexpected(Utils::Error(std::move(ok).error()));

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

	void ShaderProgram::Bind() const
	{
		glUseProgram(m_Id);
	}

	int ShaderProgram::GetUniformLocation(const std::string& name) const
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

	void ShaderProgram::SetInt32(const std::string& name, int value) const 
	{
		int location = GetUniformLocation(name);
		if (location != -1) glUniform1i(location, value);
	}

	void ShaderProgram::SetUInt32(const std::string& name, unsigned int value) const 
	{
		int location = GetUniformLocation(name);
		if (location != -1) glUniform1ui(location, value);
	}

	void ShaderProgram::SetFloat(const std::string& name, float value) const 
	{
		int location = GetUniformLocation(name);
		if (location != -1) glUniform1f(location, value);
	}

	void ShaderProgram::SetVec2(const std::string& name, const glm::vec2& vec) const 
	{
		int location = GetUniformLocation(name);
		if (location != -1) glUniform2fv(location, 1, glm::value_ptr(vec));
	}

	void ShaderProgram::SetVec3(const std::string& name, const glm::vec3& vec) const 
	{
		int location = GetUniformLocation(name);
		if (location != -1) glUniform3fv(location, 1, glm::value_ptr(vec));
	}

	void ShaderProgram::SetVec4(const std::string& name, const glm::vec4& vec) const 
	{
		int location = GetUniformLocation(name);
		if (location != -1) glUniform4fv(location, 1, glm::value_ptr(vec));
	}

	void ShaderProgram::SetMatrix3x3(const std::string& name, const glm::mat3& mat) const 
	{
		int location = GetUniformLocation(name);
		if (location != -1) glUniformMatrix3fv(location, 1, GL_FALSE, &mat[0][0]);
	}

	void ShaderProgram::SetMatrix4x4(const std::string& name, const glm::mat4& mat) const 
	{
		int location = GetUniformLocation(name);
		if (location != -1) glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
	}
}