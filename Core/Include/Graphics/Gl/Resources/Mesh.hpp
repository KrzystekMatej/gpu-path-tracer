#pragma once
#include <expected>
#include "Utils/Error/Error.hpp"
#include "IO/Model.hpp"

namespace Core::Graphics::Gl
{
	struct Mesh
	{
	public:
		Mesh(const Mesh&) = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh(Mesh&& other) noexcept
			: m_VertexArray(other.m_VertexArray),
			  m_VertexBuffer(other.m_VertexBuffer),
			  m_IndexBuffer(other.m_IndexBuffer),
			  m_VertexCount(other.m_VertexCount)
		{
			other.m_VertexArray = 0;
			other.m_VertexBuffer = 0;
			other.m_IndexBuffer = 0;
			other.m_VertexCount = 0;
		}
		Mesh& operator=(Mesh&& other) noexcept
		{
			if (this != &other)
			{
				m_VertexArray = other.m_VertexArray;
				m_VertexBuffer = other.m_VertexBuffer;
				m_IndexBuffer = other.m_IndexBuffer;
				m_VertexCount = other.m_VertexCount;
				other.m_VertexArray = 0;
				other.m_VertexBuffer = 0;
				other.m_IndexBuffer = 0;
				other.m_VertexCount = 0;
			}
			return *this;
		}

		static std::expected<Mesh, Utils::Error> Create(const IO::ParsedMesh& parsedMesh);

		~Mesh();
		void Bind() const;
		void Unbind() const;
	private:
		Mesh(uint32_t vertexArray, uint32_t vertexBuffer, uint32_t indexBuffer, uint32_t vertexCount)
			: m_VertexArray(vertexArray), m_VertexBuffer(vertexBuffer), m_IndexBuffer(indexBuffer), m_VertexCount(vertexCount) {}

		uint32_t m_VertexArray;
		uint32_t m_VertexBuffer;
		uint32_t m_IndexBuffer;
		uint32_t m_VertexCount;
	};
}