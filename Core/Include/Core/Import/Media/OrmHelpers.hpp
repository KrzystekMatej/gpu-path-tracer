#pragma once
#include <vector>
#include <Core/Import/Media/Image.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Import::Media
{
    std::expected<void, Utils::Error> ConvertRmoToOrm(Image& rmo);
    std::expected<Image, Utils::Error> PackOrmTexture(const std::optional<Image>& occlusion, float fallbackOcclusion, const Image& rm);
    std::expected<Image, Utils::Error> PackOrmTexture(
        const std::optional<Image>& occlusion,
        float fallbackOcclusion, 
        const std::optional<Image>& roughness,
        float fallbackRoughness, 
        const std::optional<Image>& metallic, 
        float fallbackMetallic);
    Image ExtractRmTexture(const Image& orm);
}