#pragma once
#include <string>
#include <Core/Utils/Error.hpp>
#include <cuda_runtime.h>

namespace Core::Graphics::Cuda::Utils
{
    std::string GetCudaErrorMessage(cudaError_t error);
    Core::Utils::Error MakeCudaError(const char* operation, cudaError_t error);
}

#define CORE_CUDA_TRY_KERNEL(operation, ...)                            \
	do {                                                                \
		__VA_ARGS__;                                                     \
		                                                                 \
		cudaError_t core_cuda_launch_error = cudaGetLastError();         \
		if (core_cuda_launch_error != cudaSuccess) {                    \
			return std::unexpected(                                      \
				Utils::MakeCudaError((operation), core_cuda_launch_error) \
			);                                                           \
		}                                                               \
		                                                                 \
		cudaError_t core_cuda_execution_error = cudaDeviceSynchronize(); \
		if (core_cuda_execution_error != cudaSuccess) {                 \
			return std::unexpected(                                      \
				Utils::MakeCudaError((operation), core_cuda_execution_error) \
			);                                                           \
		}                                                               \
	} while (false)