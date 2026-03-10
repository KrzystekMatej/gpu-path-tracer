#pragma once
#include <cstdint>
#include <expected>
#include "Utils/Error/Error.hpp"
#include "Graphics/Gl/Shader.hpp"

namespace Core::Graphics::Gl
{
	class Program
	{
	public:
		Program(const Program&) = delete;
		Program& operator=(const Program&) = delete;
		Program(Program&&) noexcept;
		Program& operator=(Program&&) noexcept;
		~Program();

		template<size_t N>
		static std::expected<Program, Utils::Error> Create(const std::array<Shader, N>& shaders)
		{
			Program program;
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

		void Bind() const;
		void Unbind() const;
	private:
		Program();
		std::expected<void, Utils::Error> Link();
		void AttachShader(const Shader& shader);
		void DetachShader(const Shader& shader);

		uint32_t m_Id = 0;
	};
}