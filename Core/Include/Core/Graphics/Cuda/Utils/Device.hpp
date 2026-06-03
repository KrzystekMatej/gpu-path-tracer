#pragma once
#include <Core/Graphics/Cuda/Utils/Error.hpp>

namespace Core::Graphics::Cuda::Utils::Device
{
    struct MemoryInfo
    {
        size_t freeBytes;
        size_t totalBytes;
    };
    
    std::expected<MemoryInfo, Core::Utils::Error> GetMemoryInfo();
}