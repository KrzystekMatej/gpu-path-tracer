#pragma once

namespace Core::Graphics::Gl
{
	struct Mesh
	{
		uint32_t vertexArray;
		uint32_t vertexBuffer;
		uint32_t indexBuffer;
		uint32_t vertexCount;
	};

	struct Texture
	{
		uint32_t id;
		uint32_t format;
		uint32_t target;
	};

	struct EnvironmentMap
	{
		Texture background;
		Texture irradianceMap;
		Texture prefilteredMap;
	};
}