#include <Core/Graphics/Cuda/Utils/Device.hpp>
#include <cuda_runtime.h>

namespace Core::Graphics::Cuda::Utils::Device
{
    std::expected<MemoryInfo, Core::Utils::Error> GetMemoryInfo()
    {
        size_t freeBytes = 0;
        size_t totalBytes = 0;
        cudaError_t result = cudaMemGetInfo(&freeBytes, &totalBytes);
        if (result != cudaSuccess)
        {
            return std::unexpected(MakeCudaError("cudaMemGetInfo", result));
        }

        return MemoryInfo{ freeBytes, totalBytes };
    }
}