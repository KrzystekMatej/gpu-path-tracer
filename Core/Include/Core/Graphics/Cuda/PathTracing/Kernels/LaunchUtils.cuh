#pragma once

#include <cstdint>

namespace Core::Graphics::Cuda::Kernels::LaunchUtils
{
    constexpr uint32_t ClearThreadsPerBlock = 512;
    constexpr uint32_t PostprocessThreadsPerBlock = 512;
    constexpr uint32_t InitializeThreadsPerBlock = 256;
    constexpr uint32_t IntersectThreadsPerBlock = 128;
    constexpr uint32_t MaterialEvalThreadsPerBlock = 128;
    constexpr uint32_t RegenerateThreadsPerBlock = 256;

    
	inline uint32_t GetBlockCount(uint32_t dataSize, uint32_t blockSize)
	{
		return (dataSize + blockSize - 1) / blockSize;
	}
}