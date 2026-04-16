#pragma once
#include <string>
#include <Core/Utils/Error.hpp>
#include <cuda_runtime.h>

namespace Core::Graphics::Cuda::Utils
{
    std::string GetCudaErrorMessage(cudaError_t error);
    Core::Utils::Error MakeCudaError(const char* operation, cudaError_t error);
}