#pragma once

namespace Core::Graphics::Cuda
{
	struct __align__(16) DeviceCamera
	{
		float3 origin;
		float3 lowerLeftCorner;
		float3 horizontal;
		float3 vertical;
	};
}