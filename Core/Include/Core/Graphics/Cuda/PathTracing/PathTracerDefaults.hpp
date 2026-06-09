#pragma once
#include <cstdint>

namespace Core::Graphics::Cuda
{
    class PathTracerDefaults
    {
        public:
            static constexpr uint32_t RussianRouletteStartDepth = 3;
            static constexpr uint32_t InvalidIndex = 0xFFFFFFFF;
            static constexpr float MinT = 1e-4f;
            static constexpr float MaxT = 1e34f;

            static constexpr uint32_t PathDepthLimit = 20;
            static constexpr uint32_t MinPathDepthLimit = 1;
            static constexpr uint32_t MaxPathDepthLimit = 100;

            static constexpr uint32_t SampleGridSize = 10;
            static constexpr uint32_t SamplesPerPixel = 1024;
            static constexpr uint32_t MinSamplesPerPixel = 1;
            static constexpr uint32_t MaxSamplesPerPixel = 10'000'000;

            static constexpr uint32_t FrameWidth = 1920;
            static constexpr uint32_t MinFrameWidth = 1;
            static constexpr uint32_t MaxFrameWidth = 7680;
            
            static constexpr uint32_t FrameHeight = 1080;
            static constexpr uint32_t MinFrameHeight = 1;
            static constexpr uint32_t MaxFrameHeight = 4320;
            
            static constexpr uint64_t RandomSeed = 1234;
    };
}