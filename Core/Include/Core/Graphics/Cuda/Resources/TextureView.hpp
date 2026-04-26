#pragma once
#include <cstdint>

namespace Core::Graphics::Cuda
{
	template<typename T>
	struct TextureView
	{
		uint64_t texture;
	};
}