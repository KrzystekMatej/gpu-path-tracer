#include "CudaHello.hpp"

#include <cuda_runtime.h>
#include <cstdio>

__global__ void hello_kernel()
{
    printf("Hello from CUDA kernel!\n");
}

void CudaHello()
{
    hello_kernel<<<1, 1>>>();
    cudaDeviceSynchronize();
}