#pragma once
#include <string>
#include <vector>
#include <optional>
#include <Core/Graphics/Common/Material.hpp>
#include <Core/Graphics/Common/Vertex.hpp>

namespace Core::Import
{
	struct ParsedMesh
	{
		uint32_t index;

		std::vector<Graphics::Vertex> vertices;
		std::vector<uint32_t> indices;

		std::optional<uint32_t> materialIndex;
	};

	struct ParsedMaterial
	{
		std::string name;

		Graphics::SurfaceModel surface;

		float albedo[3];
		float roughness;
		float metallic;

		std::optional<std::string> albedoTexture;
		std::optional<std::string> roughnessTexture;
		std::optional<std::string> metallicTexture;
		std::optional<std::string> aoTexture;
		std::optional<std::string> normalTexture;
	};

	struct ParsedModel
	{
		std::vector<ParsedMesh> meshes;
		std::vector<ParsedMaterial> materials;
	};
}
