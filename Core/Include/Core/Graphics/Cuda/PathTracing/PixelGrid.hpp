#pragma once
#include <cstdint>

namespace Core::Graphics::Cuda
{
    struct PixelGrid
    {
        uint32_t width;
        uint32_t height;
        uint32_t samplesPerPixel;
        uint32_t samplesPerPixelAxis;
    };
}