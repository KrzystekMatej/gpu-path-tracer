#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>

namespace Core::Graphics
{
	enum class ChannelLayout : uint8_t
	{
		R = 1,
		RGB = 3,
		RGBA = 4,
	};

	enum class ComponentType : uint8_t
	{
		UInt8,
		Float16,
		Float32,
	};

	enum class ColorSpace : uint8_t
	{
		Linear,
		SRGB,
	};
	
	[[nodiscard]] constexpr std::string_view ToString(ChannelLayout value) noexcept
	{
		switch (value)
		{
			case ChannelLayout::R:    return "R";
			case ChannelLayout::RGB:  return "RGB";
			case ChannelLayout::RGBA: return "RGBA";
		}

		return "Unknown";
	}

	[[nodiscard]] constexpr std::string_view ToString(ComponentType value) noexcept
	{
		switch (value)
		{
			case ComponentType::UInt8:   return "UInt8";
			case ComponentType::Float16: return "Float16";
			case ComponentType::Float32: return "Float32";
		}

		return "Unknown";
	}

	[[nodiscard]] constexpr std::string_view ToString(ColorSpace value) noexcept
	{
		switch (value)
		{
			case ColorSpace::Linear: return "Linear";
			case ColorSpace::SRGB:   return "SRGB";
		}

		return "Unknown";
	}

	struct PixelFormat
	{
		ChannelLayout layout;
		ComponentType componentType;
		ColorSpace colorSpace;


		uint32_t GetChannelCount() const
		{
			return static_cast<uint32_t>(layout);
		}

		uint32_t GetComponentSize() const
		{
			switch (componentType)
			{
				case ComponentType::UInt8:
					return 1;
				case ComponentType::Float16:
					return 2;
				case ComponentType::Float32:
					return 4;
			}

			return 0;
		}

		uint32_t GetBytesPerPixel() const
		{
			return static_cast<uint32_t>(layout) * GetComponentSize();
		}

		bool operator==(const PixelFormat&) const = default;

		std::string ToString() const
		{
			return std::string(Core::Graphics::ToString(layout)) + "_" +
           		std::string(Core::Graphics::ToString(componentType)) + "_" +
           		std::string(Core::Graphics::ToString(colorSpace));
		}
	};
}