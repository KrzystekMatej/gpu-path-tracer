#include <Core/Utils/NumberTypes.hpp>
#include <cmath>
#include <algorithm>

namespace Core::Utils::NumberTypes
{
    float NormalizeFloat(float x, float min, float max)
    {
        if (!std::isfinite(x))
            x = min;
        x = std::clamp(x, min, max);
        return (x - min) / (max - min);
    }

    uint8_t NormalizedFloatToU8(float x)
    {
        if (!std::isfinite(x))
            x = 0.0f;
        x = std::clamp(x, 0.0f, 1.0f);
        return static_cast<uint8_t>(std::lround(x * 255.0f));
    }

    uint16_t Float32ToFloat16(float value)
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