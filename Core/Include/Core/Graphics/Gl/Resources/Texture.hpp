#pragma once
#include <expected>
#include <Core/Utils/Error.hpp>
#include <Core/Import/Media/Image.hpp>

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

		static std::expected<Texture, Utils::Error> Create2D(const Import::Media::Image& image);
		static std::expected<Texture, Utils::Error> Create2DFromMipmaps(const Import::Media::ImageMipChain& mipMaps);
		static std::expected<Texture, Utils::Error> CreateCubemap(const Import::Media::Cubemap& cubemap);
		static std::expected<Texture, Utils::Error> CreateCubemapFromMipmaps(const Import::Media::CubemapMipChain& mipMaps);

		~Texture();
		uint32_t GetId() const { return m_Id; }
		uint32_t GetTarget() const { return m_Target; }
		void Bind() const;
	private:
		uint32_t m_Id = 0;
		uint32_t m_Target = 0;
	};
}