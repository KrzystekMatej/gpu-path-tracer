#pragma once
#include <optional>
#include <Core/Import/Image.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Import
{
    std::optional<Image> ConvertRgbToRgba(const Image& image);
    Image FlipVertically(const Image& image);
}