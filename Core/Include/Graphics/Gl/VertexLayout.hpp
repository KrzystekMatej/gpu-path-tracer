#pragma once
#include <vector>

namespace Core::Graphics::Gl
{
	enum class VertexAttributeRepresentation
	{
		Float,
		Int
	};



	struct VertexAttribute
	{
		uint32_t type;
		uint32_t count;
		uint32_t normalized;
		VertexAttributeRepresentation representation;

		static uint32_t GetSizeOfType(uint32_t type);
	};

	class VertexLayout
	{
	public:
		VertexLayout() : m_Stride(0) {}

		void PushFloat(uint32_t count);
		void PushUInt32(uint32_t count, VertexAttributeRepresentation representation);
		void PushUInt8(uint32_t count, VertexAttributeRepresentation representation);

		void Apply() const;

		uint32_t GetStride() const { return m_Stride; }
		const std::vector<VertexAttribute>& GetElements() const { return m_Elements; }
	private:
		std::vector<VertexAttribute> m_Elements;
		uint32_t m_Stride;
	};

}