#include <glad/gl.h>
#include "Graphics/Gl/Program.hpp"
#include <utility>

namespace Core::Graphics::Gl
{
	Program::Program()
	{
		m_Id = glCreateProgram();
	}

	Program::Program(Program&& other) noexcept
		: m_Id(std::exchange(other.m_Id, 0)) { }

	Program& Program::operator=(Program&& other) noexcept
	{
		if (this != &other)
		{
			if (m_Id)
				glDeleteProgram(m_Id);

			m_Id = std::exchange(other.m_Id, 0);
		}
		return *this;
	}

	Program::~Program()
	{
		if (m_Id)
			glDeleteProgram(m_Id);
	}

	std::expected<void, Utils::Error> Program::Link()
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

	void Program::AttachShader(const Shader& shader)
	{
		glAttachShader(m_Id, shader.m_Id);
	}

	void Program::DetachShader(const Shader& shader)
	{
		glDetachShader(m_Id, shader.m_Id);
	}

	void Program::Bind() const
	{
		glUseProgram(m_Id);
	}

	void Program::Unbind() const
	{
		glUseProgram(0);
	}
}