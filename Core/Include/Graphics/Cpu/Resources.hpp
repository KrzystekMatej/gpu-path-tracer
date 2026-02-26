#pragma once
#include <vector>
#include <cstddef>
#include "Graphics/Vertex.hpp"

namespace Core::Graphics::Cpu
{
	struct Mesh
	{
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
	};

	struct Texture
	{
		uint32_t width;
		uint32_t height;
		uint32_t format;
		uint32_t pixelType;
		std::vector<std::byte> data;
	};

	struct EnvironmentMap
	{
		Texture background;
		std::vector<float> conditionalCdf;
		std::vector<float> marginalCdf;
	};
}

