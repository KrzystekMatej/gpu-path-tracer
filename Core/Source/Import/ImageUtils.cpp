#include <Core/Import/ImageUtils.hpp>
#include <cstring>

namespace Core::Import
{
    namespace
    {
        uint16_t Float32ToFloat16Bits(float value)
        {
            uint32_t bits = 0;
            std::memcpy(&bits, &value, sizeof(bits));

            const uint32_t sign = (bits >> 31) & 0x1;
            int32_t exponent = static_cast<int32_t>((bits >> 23) & 0xff) - 127 + 15;
            uint32_t mantissa = bits & 0x7fffff;

            if (exponent <= 0)
            {
                if (exponent < -10)
                {
                    return static_cast<uint16_t>(sign << 15);
                }

                mantissa = (mantissa | 0x800000u) >> (1 - exponent);
                return static_cast<uint16_t>((sign << 15) | (mantissa >> 13));
            }

            if (exponent >= 31)
            {
                return static_cast<uint16_t>((sign << 15) | 0x7c00);
            }

            return static_cast<uint16_t>((sign << 15) | (static_cast<uint32_t>(exponent) << 10) | (mantissa >> 13));
        }
    }

    std::optional<Image> ConvertRgbToRgba(const Image& image)
    {
        using Graphics::Common::ChannelLayout;
        using Graphics::Common::ComponentType;

        if (image.format.layout == ChannelLayout::RGBA)
            return image;
        if (image.format.layout != ChannelLayout::RGB)
            return {};

        Image result;
        result.width = image.width;
        result.height = image.height;
        result.format = image.format;
        result.format.layout = ChannelLayout::RGBA;

        const uint32_t pixelCount = image.width * image.height;

        switch (image.format.componentType)
        {
            case ComponentType::UInt8:
            {
                result.data.resize(static_cast<size_t>(pixelCount) * 4);

                for (uint32_t i = 0; i < pixelCount; ++i)
                {
                    const size_t src = static_cast<size_t>(i) * 3;
                    const size_t dst = static_cast<size_t>(i) * 4;

                    result.data[dst + 0] = image.data[src + 0];
                    result.data[dst + 1] = image.data[src + 1];
                    result.data[dst + 2] = image.data[src + 2];
                    result.data[dst + 3] = 255;
                }

                return result;
            }

            case ComponentType::Float16:
            {
                result.data.resize(static_cast<size_t>(pixelCount) * 4 * sizeof(uint16_t));

                const auto* src = reinterpret_cast<const uint16_t*>(image.data.data());
                auto* dst = reinterpret_cast<uint16_t*>(result.data.data());
                const uint16_t alpha = Float32ToFloat16Bits(1.0f);

                for (uint32_t i = 0; i < pixelCount; ++i)
                {
                    const size_t s = static_cast<size_t>(i) * 3;
                    const size_t d = static_cast<size_t>(i) * 4;

                    dst[d + 0] = src[s + 0];
                    dst[d + 1] = src[s + 1];
                    dst[d + 2] = src[s + 2];
                    dst[d + 3] = alpha;
                }

                return result;
            }

            case ComponentType::Float32:
            {
                result.data.resize(static_cast<size_t>(pixelCount) * 4 * sizeof(float));

                const auto* src = reinterpret_cast<const float*>(image.data.data());
                auto* dst = reinterpret_cast<float*>(result.data.data());

                for (uint32_t i = 0; i < pixelCount; ++i)
                {
                    const size_t s = static_cast<size_t>(i) * 3;
                    const size_t d = static_cast<size_t>(i) * 4;

                    dst[d + 0] = src[s + 0];
                    dst[d + 1] = src[s + 1];
                    dst[d + 2] = src[s + 2];
                    dst[d + 3] = 1.0f;
                }

                return result;
            }
        }

        return {};
    }
}