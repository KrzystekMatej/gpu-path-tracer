#pragma once
#include <optional>
#include <Core/Import/Media/Image.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Import::Media
{
    std::optional<Image> ConvertRgbToRgba(const Image& image);
    Image FlipVertically(const Image& image);
}