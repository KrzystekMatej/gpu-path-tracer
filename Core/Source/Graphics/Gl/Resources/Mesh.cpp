#include <glad/gl.h>
#include <Core/Graphics/Gl/Resources/Mesh.hpp>
#include <Core/Graphics/Gl/Resources/VertexLayout.hpp>

namespace Core::Graphics::Gl::Resources
{
	Mesh::Mesh(uint32_t vertexCount)
		: m_VertexCount(vertexCount)
	{
		glGenVertexArrays(1, &m_VertexArray);
		glGenBuffers(1, &m_VertexBuffer);
		glGenBuffers(1, &m_IndexBuffer);
	}

	Mesh::Mesh(Mesh&& other) noexcept
		: m_VertexArray(std::exchange(other.m_VertexArray, 0)),
		m_VertexBuffer(std::exchange(other.m_VertexBuffer, 0)),
		m_IndexBuffer(std::exchange(other.m_IndexBuffer, 0)),
		m_VertexCount(other.m_VertexCount) { }

	Mesh& Mesh::operator=(Mesh&& other) noexcept
	{
		if (this != &other)
		{
			if (m_VertexArray)
				glDeleteVertexArrays(1, &m_VertexArray);
			if (m_VertexBuffer)
				glDeleteBuffers(1, &m_VertexBuffer);
			if (m_IndexBuffer)
				glDeleteBuffers(1, &m_IndexBuffer);

			m_VertexArray = std::exchange(other.m_VertexArray, 0);
			m_VertexBuffer = std::exchange(other.m_VertexBuffer, 0);
			m_IndexBuffer = std::exchange(other.m_IndexBuffer, 0);
			m_VertexCount = other.m_VertexCount;
		}
		return *this;
	}

	Mesh::~Mesh()
	{
		if (m_VertexArray)
		{
			glDeleteVertexArrays(1, &m_VertexArray);
			m_VertexArray = 0;
		}
		if (m_VertexBuffer)
		{
			glDeleteBuffers(1, &m_VertexBuffer);
			m_VertexBuffer = 0;
		}
		if (m_IndexBuffer)
		{
			glDeleteBuffers(1, &m_IndexBuffer);
			m_IndexBuffer = 0;
		}
	}

	std::expected<Mesh, Utils::Error> Mesh::Create(const Import::ParsedMesh& parsedMesh)
	{
		Mesh mesh(static_cast<uint32_t>(parsedMesh.indices.size()));
		mesh.BindVertexArray();
		mesh.BindVertexBuffer();
		glBufferData(GL_ARRAY_BUFFER, parsedMesh.vertices.size() * sizeof(Graphics::Common::Vertex), parsedMesh.vertices.data(), GL_STATIC_DRAW);
		mesh.BindIndexBuffer();
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, parsedMesh.indices.size() * sizeof(uint32_t), parsedMesh.indices.data(), GL_STATIC_DRAW);

		VertexLayout layout;
		layout.PushFloat(3);
		layout.PushFloat(3);
		layout.PushFloat(4);
		layout.PushFloat(2);
		layout.Apply();

		return mesh;
	}

	void Mesh::BindVertexArray() const
	{
		glBindVertexArray(m_VertexArray);
	}

	void Mesh::BindVertexBuffer() const
	{
		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
	}

	void Mesh::BindIndexBuffer() const
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBuffer);
	}
}
