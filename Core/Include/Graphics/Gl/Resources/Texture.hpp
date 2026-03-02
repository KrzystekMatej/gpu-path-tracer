#pragma once
#include <expected>
#include "Error/Error.hpp"
#include "IO/Image.hpp"

namespace Core::Graphics::Gl
{
	struct Texture
	{
	public:
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		Texture(Texture&&) noexcept
		{
			m_Id = 0;
			m_Target = 0;
		}
		Texture& operator=(Texture&& other) noexcept
		{
			if (this != &other)
			{
				m_Id = other.m_Id;
				m_Target = other.m_Target;
				other.m_Id = 0;
				other.m_Target = 0;
			}
			return *this;
		}

		std::expected<Texture, Error> Create2D(const IO::Image& image);
		std::expected<Texture, Error> Create2DFromMipmaps(const IO::ImageMipChain& mipMaps);
		std::expected<Texture, Error> CreateCubemap(const IO::Cubemap& cubemap);
		std::expected<Texture, Error> CreateCubemapFromMipmaps(const IO::CubemapMipChain& mipMaps);

		~Texture();
		void Bind() const;
		void Unbind() const;
	private:
		Texture(uint32_t id, uint32_t target)
			: m_Id(id), m_Target(target) {}

		uint32_t m_Id;
		uint32_t m_Target;
	};
}