#pragma once
#include <cstdint>
#include <expected>
#include <Core/Graphics/Gl/ShaderType.hpp>
#include <Core/Utils/Error.hpp>
#include <Core/Import/Shader.hpp>

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

		static std::expected<Shader, Utils::Error> Create(const Import::Shader& shader);
	private:
		friend class ShaderProgram;
		Shader(uint32_t type);

		uint32_t m_Id = 0;
		uint32_t m_Type = 0;
	};
}