#pragma once
#include <cstdint>
#include <cuda_runtime.h>

namespace Core::Graphics::Cuda
{
	template<typename T>
	class TextureView
	{
	public:
		TextureView(uint64_t texture) : m_Texture(texture) {}

		__device__ __forceinline__ T Sample(int u, int v) const
		{
			return tex2D<T>(m_Texture, u + 0.5f, v + 0.5f);
		}

		__device__ __forceinline__ T Sample(float2 uv) const
		{
			return tex2D<T>(m_Texture, uv.x, uv.y);
		}

		__device__ __forceinline__ T Sample(float u, float v) const
		{
			return tex2D<T>(m_Texture, u, v);
		}
	private:
		uint64_t m_Texture;
	};
}