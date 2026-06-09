#pragma once
#include <Core/Graphics/Cuda/Utils/Error.hpp>

namespace Core::Graphics::Cuda::Runtime
{
    std::expected<void, Core::Utils::Error> SynchronizeDevice();
    std::expected<void, Core::Utils::Error> ResetDevice();   
    
    struct MemoryInfo
    {
        size_t freeBytes;
        size_t totalBytes;
    };
    
    std::expected<MemoryInfo, Core::Utils::Error> GetMemoryInfo();
}