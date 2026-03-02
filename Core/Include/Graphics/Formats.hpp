#pragma once

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
		}

		uint32_t GetBytesPerPixel() const
		{
			return static_cast<size_t>(layout) * GetComponentSize();
		}

		bool operator==(const PixelFormat&) const = default;
	};
}