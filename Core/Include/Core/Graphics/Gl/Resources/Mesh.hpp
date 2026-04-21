#pragma once
#include <expected>
#include <Core/Utils/Error.hpp>
#include <Core/Import/Model.hpp>

namespace Core::Graphics::Gl
{
	class Mesh
	{
	public:
		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh(Mesh&& other) noexcept;
		Mesh& operator=(Mesh&& other) noexcept;
		~Mesh();

		static std::expected<Mesh, Utils::Error> Create(const Import::ParsedMesh& parsedMesh);

		uint32_t GetVertexCount() const { return m_VertexCount; }
		void BindVertexArray() const;
		void BindVertexBuffer() const;
		void BindIndexBuffer() const;
	private:
		Mesh(uint32_t vertexCount);

		uint32_t m_VertexArray = 0;
		uint32_t m_VertexBuffer = 0;
		uint32_t m_IndexBuffer = 0;
		uint32_t m_VertexCount = 0;
	};
}