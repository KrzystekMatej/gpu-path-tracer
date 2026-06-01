#pragma once
#include <cstdint>
#include <cuda_runtime.h>

namespace Core::Graphics::Cuda
{
	template<typename T>
	struct TextureView
	{
	public:
		TextureView(uint64_t texture) : m_Texture(texture) {}

		__device__ __forceinline__ T Sample(int x, int y) const
		{
			return tex2D<T>(m_Texture, x + 0.5f, y + 0.5f);
		}

		__device__ __forceinline__ T Sample(float x, float y) const
		{
			return tex2D<T>(m_Texture, x, y);
		}

	private:
		uint64_t m_Texture;
	};
}