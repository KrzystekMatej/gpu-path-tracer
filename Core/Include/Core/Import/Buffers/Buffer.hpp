#pragma once
#include <cstdint>
#include <variant>
#include <string>
#include <vector>
#include <filesystem>
#include <cassert>

namespace Core::Import
{
	struct FileBuffer
	{
		std::filesystem::path file;
		uint64_t byteOffset = 0;
	};

	struct MemoryBuffer
	{
		std::vector<uint8_t> bytes;
	};

	struct Buffer
	{
		std::string name;
		uint64_t byteLength = 0;
		std::variant<FileBuffer, MemoryBuffer> source;
	};

	enum class ElementType : uint8_t
	{
		Scalar,
		Vec2,
		Vec3,
		Vec4,
		Mat2,
		Mat3,
		Mat4
	};

	enum class ComponentType : uint16_t
	{
		Byte = 5120,
		UnsignedByte = 5121,
		Short = 5122,
		UnsignedShort = 5123,
		Int = 5124,
		UnsignedInt = 5125,
		Float = 5126,
		Double = 5130
	};

	enum class PrimitiveTopology : uint8_t
	{
		Points,
		Lines,
		LineLoop,
		LineStrip,
		Triangles,
		TriangleStrip,
		TriangleFan
	};

    constexpr uint32_t ComponentByteLength(ComponentType componentType)
    {
        switch (componentType)
        {
        case ComponentType::Byte:
        case ComponentType::UnsignedByte:
            return 1;
        case ComponentType::Short:
        case ComponentType::UnsignedShort:
            return 2;
        case ComponentType::Int:
        case ComponentType::UnsignedInt:
        case ComponentType::Float:
            return 4;
        case ComponentType::Double:
            return 8;
        }

        assert(false && "Invalid component type");
        return 0;
    }

    constexpr uint32_t ElementComponentCount(ElementType elementType)
    {
        switch (elementType)
        {
        case ElementType::Scalar:
            return 1;
        case ElementType::Vec2:
            return 2;
        case ElementType::Vec3:
            return 3;
        case ElementType::Vec4:
            return 4;
        case ElementType::Mat2:
            return 4;
        case ElementType::Mat3:
            return 9;
        case ElementType::Mat4:
            return 16;
        }

        assert(false && "Invalid element type");
        return 0;
    }
	
	constexpr uint32_t ElementByteLength(ElementType elementType, ComponentType componentType)
	{
		return ElementComponentCount(elementType) * ComponentByteLength(componentType);
	}
	
	struct BufferView
	{
		uint64_t byteOffset = 0;
		uint64_t byteStride = 0;

		uint32_t count = 0;

		ElementType elementType = ElementType::Scalar;
		ComponentType componentType = ComponentType::Float;

		bool normalized = false;
		

		uint64_t GetStride() const
		{
			uint64_t elementByteLength = ElementByteLength(elementType, componentType);
			return byteStride == 0 ? elementByteLength : byteStride;
		}
		
		uint64_t GetUsedByteLength() const
		{
			if (count == 0)
				return 0;

			return (static_cast<uint64_t>(count) - 1) * GetStride() + ElementByteLength(elementType, componentType);
		}
	};
}