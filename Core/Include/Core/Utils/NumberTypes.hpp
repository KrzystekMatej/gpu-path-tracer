#pragma once
#include <cstdint>

namespace Core::Utils::NumberTypes
{
    float NormalizeFloat(float x, float min, float max);
    uint8_t NormalizedFloatToU8(float x);
    uint16_t Float32ToFloat16(float value);
}