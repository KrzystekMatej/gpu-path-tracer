#pragma once
#include <string>
#include <Core/Utils/Error.hpp>
#include <cuda_runtime.h>

namespace Core::Graphics::Cuda::Utils
{
    std::string GetCudaErrorMessage(cudaError_t error);
    Core::Utils::Error MakeCudaError(const char* operation, cudaError_t error);
}

#define CUDA_TRY(operation, cuda_expression)                           \
	do {                                                                	\
		cudaError_t core_cuda_error = (cuda_expression);         			\
		if (core_cuda_error != cudaSuccess) {                    			\
			return std::unexpected(                                      	\
				Utils::MakeCudaError((operation), core_cuda_error) 			\
			);                                                           	\
		}                                                               	\
	} while (false)
	

#define CUDA_KERNEL_LAUNCH_CHECK(operation)                                      \
    do {                                                                         \
        cudaError_t core_cuda_launch_error = cudaGetLastError();                 \
        if (core_cuda_launch_error != cudaSuccess) {                             \
            return std::unexpected(                                              \
                Utils::MakeCudaError((operation), core_cuda_launch_error)        \
            );                                                                   \
        }                                                                        \
    } while (false)

#define CUDA_KERNEL_SYNC_CHECK(operation)                                        \
    do {                                                                         \
        cudaError_t core_cuda_sync_error = cudaDeviceSynchronize();              \
        if (core_cuda_sync_error != cudaSuccess) {                               \
            return std::unexpected(                                              \
                Utils::MakeCudaError((operation), core_cuda_sync_error)          \
            );                                                                   \
        }                                                                        \
    } while (false)

#define CUDA_TRY_KERNEL_LAUNCH(operation, ...)                                   \
    do {                                                                         \
        __VA_ARGS__;                                                             \
        CUDA_KERNEL_LAUNCH_CHECK(operation);                                     \
    } while (false)

#ifdef CORE_DEBUG

#define CUDA_TRY_KERNEL_DEBUG(operation, ...)                                    \
    do {                                                                         \
        __VA_ARGS__;                                                             \
        CUDA_KERNEL_LAUNCH_CHECK(operation);                                     \
        CUDA_KERNEL_SYNC_CHECK(operation);                                       \
    } while (false)

#else

#define CUDA_TRY_KERNEL_DEBUG(operation, ...)                                    \
    do {                                                                         \
        __VA_ARGS__;                                                             \
        CUDA_KERNEL_LAUNCH_CHECK(operation);                                     \
    } while (false)

#endif