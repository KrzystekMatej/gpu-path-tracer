#include <Core/Graphics/Cuda/Runtime/Global.hpp>

namespace Core::Graphics::Cuda::Runtime
{
    std::expected<void, Core::Utils::Error> SynchronizeDevice()
    {
        CUDA_TRY("cudaDeviceSynchronize", cudaDeviceSynchronize());
        return {};
    }

    std::expected<void, Core::Utils::Error> ResetDevice()
    {
        CUDA_TRY("cudaDeviceReset", cudaDeviceReset());
        return {};
    }
    
    std::expected<MemoryInfo, Core::Utils::Error> GetMemoryInfo()
    {
        size_t freeBytes = 0;
        size_t totalBytes = 0;
        CUDA_TRY("cudaMemGetInfo", cudaMemGetInfo(&freeBytes, &totalBytes));
        return MemoryInfo{ freeBytes, totalBytes };
    }
}