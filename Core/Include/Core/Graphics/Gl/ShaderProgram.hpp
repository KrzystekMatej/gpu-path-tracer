#pragma once
#include <cstdint>
#include <expected>
#include <span>
#include <unordered_map>
#include <Core/External/Glm.hpp>
#include <Core/Utils/Error.hpp>
#include <Core/Graphics/Gl/Shader.hpp>
#include <Core/Graphics/Gl/Resources/Texture.hpp>

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

		static std::expected<ShaderProgram, Utils::Error> Create(std::span<const Shader> shaders);
		void Bind();

		int GetUniformLocation(const std::string& name);
		void SetInt32(const std::string& name, int value);
		void SetUInt32(const std::string& name, unsigned int value);
		void SetFloat(const std::string& name, float value);
		void SetVec2(const std::string& name, const glm::vec2& vec);
		void SetVec3(const std::string& name, const glm::vec3& vec);
		void SetVec4(const std::string& name, const glm::vec4& vec);
		void SetMatrix3x3(const std::string& name, const glm::mat3& mat);
		void SetMatrix4x4(const std::string& name, const glm::mat4& mat);
		void SetTexture(const std::string& name, const Texture& texture);
	private:
		ShaderProgram();
		std::expected<void, Utils::Error> Link();
		void AttachShader(const Shader& shader);
		void DetachShader(const Shader& shader);

		uint32_t m_Id = 0;
		uint32_t m_TextureUnitCounter = 0;
		std::unordered_map<std::string, int> m_UniformLocationCache;
	};
}