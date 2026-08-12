#include <Core/Import/Media/ImageUtils.hpp>
#include <cstring>
#include <Core/Utils/NumberTypes.hpp>

namespace Core::Import::Media
{
    std::optional<Image> ConvertRgbToRgba(const Image& image)
    {
        using Graphics::ChannelLayout;
        using Graphics::ComponentType;

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

                for (uint32_t i = 0; i < pixelCount; i++)
                {
                    const size_t s = static_cast<size_t>(i) * 3;
                    const size_t d = static_cast<size_t>(i) * 4;

                    result.data[d + 0] = image.data[s + 0];
                    result.data[d + 1] = image.data[s + 1];
                    result.data[d + 2] = image.data[s + 2];
                    result.data[d + 3] = 255;
                }

                return result;
            }

            case ComponentType::Float16:
            {
                result.data.resize(static_cast<size_t>(pixelCount) * 4 * sizeof(uint16_t));

                const auto* source = reinterpret_cast<const uint16_t*>(image.data.data());
                auto* destination = reinterpret_cast<uint16_t*>(result.data.data());
                const uint16_t alpha = Utils::NumberTypes::Float32ToFloat16(1.0f);

                for (uint32_t i = 0; i < pixelCount; i++)
                {
                    const size_t s = static_cast<size_t>(i) * 3;
                    const size_t d = static_cast<size_t>(i) * 4;

                    destination[d + 0] = source[s + 0];
                    destination[d + 1] = source[s + 1];
                    destination[d + 2] = source[s + 2];
                    destination[d + 3] = alpha;
                }

                return result;
            }

            case ComponentType::Float32:
            {
                result.data.resize(static_cast<size_t>(pixelCount) * 4 * sizeof(float));

                const auto* source = reinterpret_cast<const float*>(image.data.data());
                auto* destination = reinterpret_cast<float*>(result.data.data());

                for (uint32_t i = 0; i < pixelCount; i++)
                {
                    const size_t s = static_cast<size_t>(i) * 3;
                    const size_t d = static_cast<size_t>(i) * 4;

                    destination[d + 0] = source[s + 0];
                    destination[d + 1] = source[s + 1];
                    destination[d + 2] = source[s + 2];
                    destination[d + 3] = 1.0f;
                }

                return result;
            }
        }

        return {};
    }

    Image FlipVertically(const Image& image)
    {
        const size_t bytesPerPixel = image.format.GetBytesPerPixel();
        const size_t rowSize = static_cast<size_t>(image.width) * bytesPerPixel;
        assert(image.data.size() == rowSize * image.height);

        Image result;
        result.width = image.width;
        result.height = image.height;
        result.format = image.format;
        result.data.resize(image.data.size());

        for (uint32_t y = 0; y < image.height; ++y)
        {
            const size_t sourceOffset = static_cast<size_t>(image.height - 1 - y) * rowSize;
            const size_t destinationOffset = static_cast<size_t>(y) * rowSize;

            std::memcpy(result.data.data() + destinationOffset, image.data.data() + sourceOffset, rowSize);
        }

        return result;
    }
}