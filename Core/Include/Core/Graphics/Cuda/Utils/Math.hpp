
namespace Core::Graphics::Cuda::Utils::Math
{
	inline __forceinline__ __host__ __device__ float At(float2 number, int index)
	{
		switch (index)
		{
			case 0:
				return number.x;
			case 1:
				return number.y;
			default:
				return 0.0f;
		}
	}

	inline __forceinline__ __host__ __device__ float At(float3 number, int index)
	{
		switch (index)
		{
			case 0:
				return number.x;
			case 1:
				return number.y;
			case 2:
				return number.z;
			default:
				return 0.0f;
		}
	}

	inline __forceinline__ __host__ __device__ float At(float4 number, int index)
	{
		switch (index)
		{
			case 0:
				return number.x;
			case 1:
				return number.y;
			case 2:
				return number.z;
			case 3:
				return number.w;
			default:
				return 0.0f;
		}
	}
}