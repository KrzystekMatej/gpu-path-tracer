#include <glad/gl.h>
#include <Core/Graphics/Gl/Resources/VertexLayout.hpp>

namespace Core::Graphics::Gl::Resources
{
	uint32_t VertexAttribute::GetSizeOfType(uint32_t type)
	{
		switch (type)
		{
			case GL_FLOAT: return sizeof(float);
			case GL_UNSIGNED_INT: return sizeof(uint32_t);
			case GL_UNSIGNED_BYTE: return sizeof(uint8_t);
		}
		return 0;
	}

	void VertexLayout::PushFloat(uint32_t count)
	{
		m_Elements.push_back({ GL_FLOAT, count, GL_FALSE, VertexAttributeRepresentation::Float });
		m_Stride += count * VertexAttribute::GetSizeOfType(GL_FLOAT);
	}

	void VertexLayout::PushUInt32(uint32_t count, VertexAttributeRepresentation representation)
	{
		m_Elements.push_back({ GL_UNSIGNED_INT, count, GL_FALSE, representation });
		m_Stride += count * VertexAttribute::GetSizeOfType(GL_UNSIGNED_INT);
	}

	void VertexLayout::PushUInt8(uint32_t count, VertexAttributeRepresentation representation)
	{
		m_Elements.push_back({ GL_UNSIGNED_BYTE, count, GL_TRUE, representation });
		m_Stride += count * VertexAttribute::GetSizeOfType(GL_UNSIGNED_BYTE);
	}

	void VertexLayout::Apply() const
	{
		size_t offset = 0;
		for (size_t i = 0; i < m_Elements.size(); i++)
		{
			const auto& element = m_Elements[i];
			glEnableVertexAttribArray(static_cast<uint32_t>(i));

			if (element.representation == VertexAttributeRepresentation::Int)
			{
				glVertexAttribIPointer(static_cast<uint32_t>(i), element.count, element.type, m_Stride, reinterpret_cast<void*>(offset));
			}
			else 
			{
				glVertexAttribPointer(static_cast<uint32_t>(i), element.count, element.type, element.normalized, m_Stride, reinterpret_cast<void*>(offset));
			}

			offset += element.count * VertexAttribute::GetSizeOfType(element.type);
		}
	}
}