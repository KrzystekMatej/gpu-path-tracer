#pragma once
#include <vector>
#include <expected>
#include <Core/Graphics/Common/Vertex.hpp>
#include <Core/Import/Model.hpp>

namespace Core::Graphics::Cpu::Resources
{
	class Mesh
	{
	public:
		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh(Mesh&&) noexcept = default;
		Mesh& operator=(Mesh&&) noexcept = default;

		static Mesh Create(Import::ParsedMesh parsedMesh)
		{
			Mesh mesh(std::move(parsedMesh.vertices), std::move(parsedMesh.indices));
			return mesh;
		}

		const std::vector<Common::Vertex>& GetVertices() const { return m_Vertices; }
		const std::vector<uint32_t>& GetIndices() const { return m_Indices; }
	private:
		friend void ComputeTangents(Mesh&);

		Mesh(std::vector<Common::Vertex> vertices, std::vector<uint32_t> indices)
			: m_Vertices(std::move(vertices)), m_Indices(std::move(indices)) {}

		std::vector<Common::Vertex> m_Vertices;
		std::vector<uint32_t> m_Indices;
	};
}
