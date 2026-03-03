#pragma once
#include <vector>
#include <expected>
#include "IO/Image.hpp"

namespace Core::Graphics::Cpu
{
	class Texture
	{
	public:
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		Texture(Texture&&) noexcept = default;
		Texture& operator=(Texture&&) noexcept = default;

		static Texture Create(IO::Image image)
		{
			return Texture(image.width, image.height, image.format, std::move(image.data));
		}

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
		const PixelFormat& GetFormat() const { return m_Format; }
	private:
		Texture(uint32_t width, uint32_t height, PixelFormat format, std::vector<uint8_t> data)
			: m_Width(width), m_Height(height), m_Format(format), m_Data(std::move(data)) {}

		uint32_t m_Width;
		uint32_t m_Height;
		PixelFormat m_Format;
		std::vector<uint8_t> m_Data;
	};
}
