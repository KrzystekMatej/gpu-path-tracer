#include <glad/gl.h>
#include "Graphics/Gl/Resources/Mesh.hpp"
#include "Graphics/Gl/VertexLayout.hpp"

namespace Core::Graphics::Gl
{
	std::expected<Mesh, Utils::Error> Mesh::Create(const IO::ParsedMesh& parsedMesh)
	{
		uint32_t vertexArray;
		glGenVertexArrays(1, &vertexArray);
		glBindVertexArray(vertexArray);
		uint32_t vertexBuffer;
		glGenBuffers(1, &vertexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, parsedMesh.vertices.size() * sizeof(Graphics::Vertex), parsedMesh.vertices.data(), GL_STATIC_DRAW);
		uint32_t indexBuffer;
		glGenBuffers(1, &indexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, parsedMesh.indices.size() * sizeof(uint32_t), parsedMesh.indices.data(), GL_STATIC_DRAW);

		VertexLayout layout;
		layout.PushFloat(3);
		layout.PushFloat(3);
		layout.PushFloat(4);
		layout.PushFloat(2);
		layout.Apply();

		return Mesh(vertexArray, vertexBuffer, indexBuffer, static_cast<uint32_t>(parsedMesh.indices.size()));
	}

	Mesh::~Mesh()
	{
		if (m_VertexArray)
			glDeleteVertexArrays(1, &m_VertexArray);
		if (m_VertexBuffer)
			glDeleteBuffers(1, &m_VertexBuffer);
		if (m_IndexBuffer)
			glDeleteBuffers(1, &m_IndexBuffer);
	}
	void Mesh::Bind() const
	{
		glBindVertexArray(m_VertexArray);
	}
	void Mesh::Unbind() const
	{
		glBindVertexArray(0);
	}
}