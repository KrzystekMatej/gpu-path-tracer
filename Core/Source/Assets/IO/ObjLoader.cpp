#include "Assets/IO/ObjLoader.hpp"
#include <map>
#include <unordered_map>
#include <format>
#include <tiny_obj_loader.h>

namespace Core::Assets::IO
{
	namespace
	{
		struct ObjIndexKey
		{
			int32_t v;
			int32_t vt;
			int32_t vn;

			friend bool operator==(const ObjIndexKey& a, const ObjIndexKey& b) noexcept
			{
				return a.v == b.v && a.vt == b.vt && a.vn == b.vn;
			}
		};

		struct ObjIndexKeyHash
		{
			static size_t Combine(size_t h, uint32_t x) noexcept
			{
				return h ^ (static_cast<size_t>(x) + 0x9e3779b97f4a7c15ull + (h << 6) + (h >> 2));
			}

			size_t operator()(const ObjIndexKey& k) const noexcept
			{
				size_t h = 0;
				h = Combine(h, static_cast<uint32_t>(k.v));
				h = Combine(h, static_cast<uint32_t>(k.vt));
				h = Combine(h, static_cast<uint32_t>(k.vn));
				return h;
			}
		};

		struct SubmeshBuilder
		{
			std::vector<Graphics::Vertex> vertices;
			std::vector<uint32_t> indices;
			std::unordered_map<ObjIndexKey, size_t, ObjIndexKeyHash> indexMap;
		};

		const std::string ShadingModelKey = "shader";
		const std::string ConstantShaderValue = "constant";
		const std::string NormalShaderValue = "normal";
		const std::string LambertShaderValue = "lambert";
		const std::string TorranceShaderValue = "torrance";
	}

	std::expected<ParsedModel, Error> Load(const std::filesystem::path& path)
	{
		tinyobj::ObjReaderConfig readerConfig;
		readerConfig.mtl_search_path = "./";
		readerConfig.triangulate = true;
		readerConfig.vertex_color = false;

		tinyobj::ObjReader reader;

		if (!reader.ParseFromFile(path.string(), readerConfig))
		{
		  if (!reader.Error().empty())
		  {
			  return std::unexpected(Error(std::format("TinyObjReader: {}", reader.Error())));
		  }
		  return std::unexpected(Error(std::format("TinyObjReader failed to load obj '{}'", path.string())));
		}

		auto& attrib = reader.GetAttrib();
		auto& shapes = reader.GetShapes();

		constexpr size_t fv = 3;

		std::vector<ParsedMesh> meshes;

		for (size_t s = 0; s < shapes.size(); s++)
		{
			size_t index_offset = 0;
			std::map<uint32_t, SubmeshBuilder> submeshes;

			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
			{
				int mat_id = shapes[s].mesh.material_ids[f];
				auto& builder = submeshes[mat_id];

				for (size_t v = 0; v < fv; v++)
				{
					Graphics::Vertex vertex{};

					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					vertex.position.x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
					vertex.position.y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
					vertex.position.z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

					if (idx.normal_index >= 0)
					{
						vertex.normal.x = attrib.normals[3 * size_t(idx.normal_index) + 0];
						vertex.normal.y = attrib.normals[3 * size_t(idx.normal_index) + 1];
						vertex.normal.z = attrib.normals[3 * size_t(idx.normal_index) + 2];
					}
					else return std::unexpected(Error("Mesh loading failed: normal data is required but not found!"));

					vertex.normal = glm::normalize(vertex.normal);

					if (idx.texcoord_index >= 0)
					{
						vertex.uv.x = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
						vertex.uv.y = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
					}
					else return std::unexpected(Error("Mesh loading failed: uv data is required but not found!"));

					ObjIndexKey key
					{
						static_cast<int32_t>(idx.vertex_index),
						static_cast<int32_t>(idx.texcoord_index),
						static_cast<int32_t>(idx.normal_index)
					};

					auto [it, inserted] = builder.indexMap.try_emplace(key, static_cast<uint32_t>(builder.vertices.size()));

					if (inserted)
						builder.vertices.push_back(vertex);

					builder.indices.push_back(it->second);
				}
				index_offset += fv;
			}

			for (auto& [mat_id, builder] : submeshes)
			{
				meshes.push_back(ParsedMesh
				{
					.index = static_cast<uint32_t>(meshes.size()),
					.vertices = std::move(builder.vertices),
					.indices = std::move(builder.indices),
					.materialIndex = mat_id >= 0
						? std::make_optional<uint32_t>(mat_id)
						: std::nullopt
				});
			}
		}
		
		auto& materialSources = reader.GetMaterials();
		
		std::vector<ParsedMaterial> materials;

		for (auto& source : materialSources)
		{
			auto it = source.unknown_parameter.find(ShadingModelKey);
			Graphics::ShadingModel shader = Graphics::ShadingModel::Lambert;

			if (it != source.unknown_parameter.end())
			{
				const std::string& shaderValue = it->second;
				if (shaderValue == ConstantShaderValue)
					shader = Graphics::ShadingModel::Constant;
				else if (shaderValue == NormalShaderValue)
					shader = Graphics::ShadingModel::Normal;
				else if (shaderValue == TorranceShaderValue)
					shader = Graphics::ShadingModel::TorranceSparrow;
			}

			materials.push_back(ParsedMaterial{
				.name = source.name,
				.shader = shader,
				.albedo = { source.diffuse[0], source.diffuse[1], source.diffuse[2] },
				.roughness = source.roughness,
				.metallic = source.metallic,
				.albedoTexture = source.diffuse_texname.empty() ? std::nullopt : std::make_optional(source.diffuse_texname),
				.roughnessTexture = source.roughness_texname.empty() ? std::nullopt : std::make_optional(source.roughness_texname),
				.metallicTexture = source.metallic_texname.empty() ? std::nullopt : std::make_optional(source.metallic_texname),
				.aoTexture = source.ambient_texname.empty() ? std::nullopt : std::make_optional(source.ambient_texname),
				.normalTexture = source.normal_texname.empty() ? std::nullopt : std::make_optional(source.normal_texname)
			});
		}


		return ParsedModel
		{
			.meshes = std::move(meshes),
			.materials = std::move(materials)
		};
	}
}