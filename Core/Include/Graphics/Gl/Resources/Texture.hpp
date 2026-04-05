#pragma once
#include <expected>
#include "Utils/Error/Error.hpp"
#include "IO/Image.hpp"

namespace Core::Graphics::Gl
{
	class Texture
	{
	public:
		Texture(uint32_t target);
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		Texture(Texture&& other) noexcept;
		Texture& operator=(Texture&& other) noexcept;

		static std::expected<Texture, Utils::Error> Create2D(const IO::Image& image);
		static std::expected<Texture, Utils::Error> Create2DFromMipmaps(const IO::ImageMipChain& mipMaps);
		static std::expected<Texture, Utils::Error> CreateCubemap(const IO::Cubemap& cubemap);
		static std::expected<Texture, Utils::Error> CreateCubemapFromMipmaps(const IO::CubemapMipChain& mipMaps);

		~Texture();
		uint32_t GetId() const { return m_Id; }
		uint32_t GetTarget() const { return m_Target; }
		void Bind() const;
	private:
		uint32_t m_Id = 0;
		uint32_t m_Target = 0;
	};
}