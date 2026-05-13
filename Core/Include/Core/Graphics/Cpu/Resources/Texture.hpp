#pragma once
#include <cassert>
#include <vector>
#include <span>
#include <expected>
#include <Core/Import/Image.hpp>

namespace Core::Graphics::Cpu
{
	class Texture
	{
	public:
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
		Texture(Texture&&) noexcept = default;
		Texture& operator=(Texture&&) noexcept = default;

		static Texture Create(Import::Image image)
		{
			return Texture(image.width, image.height, image.format, std::move(image.data));
		}

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
		const PixelFormat& GetFormat() const { return m_Format; }

		template<typename T>
		const std::span<const T>& GetPixelView() const
		{
			assert(sizeof(T) == m_Format.GetBytesPerPixel() && "Pixel type does not match texture format");
			return std::span<const T>(reinterpret_cast<const T*>(m_Data.data()), m_Data.size() / sizeof(T));
		}
		
		template<typename T>
		T At(uint32_t x, uint32_t y) const
		{
			assert(sizeof(T) == m_Format.GetBytesPerPixel() && "Pixel type does not match texture format");
			assert(x < m_Width && y < m_Height && "Pixel coordinates out of bounds");
			uint32_t index = (y * m_Width + x) * m_Format.GetBytesPerPixel();

			T pixelValue;
			std::memcpy(&pixelValue, m_Data.data() + index, sizeof(T));
			return pixelValue;
		}
		
		template<typename T>
		T At(uint32_t x, uint32_t y, uint32_t channel) const
		{
			assert(sizeof(T) == m_Format.GetComponentSize() && "Pixel type does not match texture format");
			assert(x < m_Width && y < m_Height && "Pixel coordinates out of bounds");
			assert(channel < m_Format.GetChannelCount() && "Channel index out of bounds");
			uint32_t index = (y * m_Width + x) * m_Format.GetBytesPerPixel() + channel * sizeof(T);

			T channelValue;
			std::memcpy(&channelValue, m_Data.data() + index, sizeof(T));
			return channelValue;
		}
	private:
		Texture(uint32_t width, uint32_t height, PixelFormat format, std::vector<uint8_t> data)
			: m_Width(width), m_Height(height), m_Format(format), m_Data(std::move(data)) {}

		uint32_t m_Width;
		uint32_t m_Height;
		PixelFormat m_Format;
		std::vector<uint8_t> m_Data;
	};
}
