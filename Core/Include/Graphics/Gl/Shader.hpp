#pragma once
#include <cstdint>
#include <expected>
#include "Graphics/Gl/ShaderType.hpp"
#include "Utils/Error/Error.hpp"
#include "IO/Shader.hpp"

namespace Core::Graphics::Gl
{
	class Shader
	{
	public:
		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;
		Shader(Shader&&) noexcept;
		Shader& operator=(Shader&&) noexcept;
		~Shader();

		static std::expected<Shader, Utils::Error> Create(const IO::Shader& shader);
	private:
		friend class Program;
		Shader(uint32_t type);

		uint32_t m_Id = 0;
		uint32_t m_Type = 0;
	};
}