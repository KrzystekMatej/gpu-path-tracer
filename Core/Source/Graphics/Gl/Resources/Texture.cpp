#include <glad/gl.h>
#include "Graphics/Gl/Resources/Texture.hpp"

namespace Core::Graphics::Gl
{
	namespace
	{
		std::expected<uint32_t, Error> GetInternalFormat(const PixelFormat& format)
		{
			if (format.layout == ChannelLayout::R && format.colorSpace == ColorSpace::Linear && format.componentType == ComponentType::UInt8)
				return GL_R8;
			if (format.layout == ChannelLayout::R && format.colorSpace == ColorSpace::SRGB && format.componentType == ComponentType::UInt8)
				return GL_R8;
			if (format.layout == ChannelLayout::RGB && format.colorSpace == ColorSpace::Linear && format.componentType == ComponentType::UInt8)
				return GL_RGB8;
			if (format.layout == ChannelLayout::RGB && format.colorSpace == ColorSpace::SRGB && format.componentType == ComponentType::UInt8)
				return GL_SRGB8;
			if (format.layout == ChannelLayout::RGB && format.colorSpace == ColorSpace::Linear && format.componentType == ComponentType::Float16)
				return GL_RGB16F;
			if (format.layout == ChannelLayout::RGB && format.colorSpace == ColorSpace::Linear && format.componentType == ComponentType::Float32)
				return GL_RGB32F;
			if (format.layout == ChannelLayout::RGBA && format.colorSpace == ColorSpace::Linear && format.componentType == ComponentType::UInt8)
				return GL_RGBA8;
			if (format.layout == ChannelLayout::RGBA && format.colorSpace == ColorSpace::SRGB && format.componentType == ComponentType::UInt8)
				return GL_SRGB8_ALPHA8;
			if (format.layout == ChannelLayout::RGBA && format.colorSpace == ColorSpace::Linear && format.componentType == ComponentType::Float16)
				return GL_RGBA16F;
			if (format.layout == ChannelLayout::RGBA && format.colorSpace == ColorSpace::Linear && format.componentType == ComponentType::Float32)
				return GL_RGBA32F;

			return std::unexpected(Error(
					"Unsupported pixel format: layout {}, color space {}", 
					static_cast<uint32_t>(format.layout),
					static_cast<uint32_t>(format.colorSpace)));
		}


		uint32_t GetExternalFormat(const PixelFormat& format)
		{
			switch (format.layout)
			{
				case ChannelLayout::R:
					return GL_RED;
				case ChannelLayout::RGB:
					return GL_RGB;
				case ChannelLayout::RGBA:
					return GL_RGBA;
				default:
					return 0;
			}
		}


		uint32_t GetPixelType(const PixelFormat& format)
		{
			switch (format.componentType)
			{
				case ComponentType::UInt8:
					return GL_UNSIGNED_BYTE;
				case ComponentType::Float16:
					return GL_HALF_FLOAT;
				case ComponentType::Float32:
					return GL_FLOAT;
				default:
					return 0;
			}
		}

		uint32_t GetMipmapLevelCount(uint32_t width, uint32_t height)
		{
			return 1 + static_cast<uint32_t>(std::floor(std::log2(std::max(width, height))));
		}
	}



	std::expected<Texture, Error> Texture::Create2D(const IO::Image& image)
	{
		uint32_t target = GL_TEXTURE_2D;
		auto internalFormatResult = GetInternalFormat(image.format);
		if (!internalFormatResult)
			return std::unexpected(internalFormatResult.error());
		uint32_t internalFormat = internalFormatResult.value();
		uint32_t externalFormat = GetExternalFormat(image.format);
		uint32_t pixelType = GetPixelType(image.format);
		uint32_t id;

		glGenTextures(1, &id);
		glBindTexture(target, id);
		glTexImage2D(target, 0, internalFormat, image.width, image.height, 0, externalFormat, pixelType, image.data.data());

		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		uint32_t mipmapLevels = GetMipmapLevelCount(image.width, image.height);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmapLevels - 1);
		if (mipmapLevels > 1)
		{
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(target);
		}
		else
		{
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(target, 0);
		return Texture(id, target);
	}

	std::expected<Texture, Error> Texture::Create2DFromMipmaps(const IO::ImageMipChain& mipChain)
	{
		const auto& mipMaps = mipChain.mipMaps;
		if (mipMaps.empty())
			return std::unexpected(Error("Mipmap list cannot be empty"));

		uint32_t target = GL_TEXTURE_2D;
		auto internalFormatResult = GetInternalFormat(mipChain.format);
		if (!internalFormatResult)
			return std::unexpected(internalFormatResult.error());
		uint32_t internalFormat = internalFormatResult.value();
		uint32_t externalFormat = GetExternalFormat(mipChain.format);
		uint32_t pixelType = GetPixelType(mipChain.format);
		uint32_t id;

		glGenTextures(1, &id);
		glBindTexture(target, id);
		for (size_t i = 0; i < mipMaps.size(); i++)
		{
			const auto& mipmap = mipMaps[i];
			glTexImage2D(target, static_cast<int>(i), internalFormat, mipmap.width, mipmap.height, 0, externalFormat, pixelType, mipmap.data.data());
		}

		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		uint32_t mipmapLevels = static_cast<uint32_t>(mipMaps.size());
		glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(mipMaps.size() - 1));
		if (mipmapLevels > 1)
		{
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(target);
		}
		else
		{
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(target, 0);
		return Texture(id, target);
	}

	std::expected<Texture, Error> Texture::CreateCubemap(const IO::Cubemap& cubemap)
	{
		uint32_t target = GL_TEXTURE_CUBE_MAP;
		auto internalFormatResult = GetInternalFormat(cubemap.format);
		if (!internalFormatResult)
			return std::unexpected(internalFormatResult.error());
		uint32_t internalFormat = internalFormatResult.value();
		uint32_t externalFormat = GetExternalFormat(cubemap.format);
		uint32_t pixelType = GetPixelType(cubemap.format);
		uint32_t id;

		glGenTextures(1, &id);
		glBindTexture(target, id);
		for (size_t i = 0; i < cubemap.faces.size(); i++)
		{
			const auto& face = cubemap.faces[i];
			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<int>(i),
				0,
				internalFormat,
				cubemap.size,
				cubemap.size,
				0,
				externalFormat,
				pixelType,
				face.data());
		}

		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		uint32_t mipmapLevels = GetMipmapLevelCount(cubemap.size, cubemap.size);
		glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, mipmapLevels - 1);
		if (mipmapLevels > 1)
		{
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(target);
		}
		else
		{
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(target, 0);
		return Texture(id, target);
	}

	std::expected<Texture, Error> Texture::CreateCubemapFromMipmaps(const IO::CubemapMipChain& mipChain)
	{
		const auto& mipMaps = mipChain.mipMaps;
		if (mipMaps.empty())
			return std::unexpected(Error("Mipmap list cannot be empty"));

		uint32_t target = GL_TEXTURE_CUBE_MAP;
		auto internalFormatResult = GetInternalFormat(mipChain.format);
		if (!internalFormatResult)
			return std::unexpected(internalFormatResult.error());
		uint32_t internalFormat = internalFormatResult.value();
		uint32_t externalFormat = GetExternalFormat(mipChain.format);
		uint32_t pixelType = GetPixelType(mipChain.format);
		uint32_t id;

		glGenTextures(1, &id);
		glBindTexture(target, id);
		for (size_t i = 0; i < mipMaps.size(); i++)
		{
			const auto& mipMap = mipMaps[i];
			for (size_t j = 0; j < mipMap.faces.size(); j++)
			{
				const auto& face = mipMap.faces[j];
				glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + static_cast<int>(j),
					static_cast<int>(i),
					internalFormat,
					mipMap.size,
					mipMap.size,
					0,
					externalFormat,
					pixelType,
					face.data());
			}
		}
		glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		uint32_t mipmapLevels = static_cast<uint32_t>(mipMaps.size());
		glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, static_cast<GLint>(mipMaps.size() - 1));
		if (mipmapLevels > 1)
		{
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glGenerateMipmap(target);
		}
		else
		{
			glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		}
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(target, 0);
		return Texture(id, target);
	}

	void Texture::Bind() const
	{
		glBindTexture(m_Target, m_Id);
	}

	void Texture::Unbind() const
	{
		glBindTexture(m_Target, 0);
	}

	Texture::~Texture()
	{
		if (m_Id)
			glDeleteTextures(1, &m_Id);
	}
}