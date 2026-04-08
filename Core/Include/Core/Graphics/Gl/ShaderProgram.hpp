#pragma once
#include <cstdint>
#include <expected>
#include <span>
#include <unordered_map>
#include <Core/External/Glm.hpp>
#include <Core/Utils/Error.hpp>
#include <Core/Graphics/Gl/Shader.hpp>

namespace Core::Graphics::Gl
{
	class ShaderProgram
	{
	public:
		ShaderProgram(const ShaderProgram&) = delete;
		ShaderProgram& operator=(const ShaderProgram&) = delete;
		ShaderProgram(ShaderProgram&&) noexcept;
		ShaderProgram& operator=(ShaderProgram&&) noexcept;
		~ShaderProgram();

		static std::expected<ShaderProgram, Utils::Error> Create(std::span<Shader> shaders);
		void Bind() const;

		int GetUniformLocation(const std::string& name) const;
		void SetInt32(const std::string& name, int value) const;
		void SetUInt32(const std::string& name, unsigned int value) const;
		void SetFloat(const std::string& name, float value) const;
		void SetVec2(const std::string& name, const glm::vec2& vec) const;
		void SetVec3(const std::string& name, const glm::vec3& vec) const;
		void SetVec4(const std::string& name, const glm::vec4& vec) const;
		void SetMatrix3x3(const std::string& name, const glm::mat3& mat) const;
		void SetMatrix4x4(const std::string& name, const glm::mat4& mat) const;
	private:
		ShaderProgram();
		std::expected<void, Utils::Error> Link();
		void AttachShader(const Shader& shader);
		void DetachShader(const Shader& shader);

		uint32_t m_Id = 0;
		mutable std::unordered_map<std::string, int> m_UniformLocationCache;
	};
}