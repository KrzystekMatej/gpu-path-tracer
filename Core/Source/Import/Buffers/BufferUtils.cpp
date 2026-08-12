#include <Core/Import/Buffers/BufferUtils.hpp>

#include <fstream>

namespace Core::Import
{
    namespace
	{
		template<typename T>
		T ReadUnaligned(const uint8_t* bytes)
		{
			T value{};
			std::memcpy(&value, bytes, sizeof(T));
			return value;
		}

		template<typename T>
		void WriteUnaligned(uint8_t* bytes, T value)
		{
			std::memcpy(bytes, &value, sizeof(T));
		}

		float NormalizeSigned(float value, float maximum)
		{
			return std::max(value / maximum, -1.0f);
		}

		std::expected<void, Utils::Error> ValidateView(std::span<uint8_t> buffer, const BufferView& view, std::string_view name)
		{
			if (view.count == 0)
				return {};

			uint64_t elementByteLength = ElementByteLength(view.elementType, view.componentType);
			uint64_t stride = view.GetStride();

			if (elementByteLength == 0)
				return std::unexpected(Utils::Error("{} view element byte length is zero", name));

			if (stride < elementByteLength)
				return std::unexpected(Utils::Error("{} view stride is smaller than element byte length", name));

			const uint64_t usedByteLength = view.GetUsedByteLength();

			if (view.byteOffset > buffer.size() || usedByteLength > buffer.size() - view.byteOffset)
			{
				return std::unexpected(Utils::Error(
					"{} view is outside buffer bounds. Offset: {}, required length: {}, buffer length: {}",
					name,
					view.byteOffset,
					usedByteLength,
					buffer.size()));
			}

			return {};
		}

		bool IsInteger(ComponentType componentType)
		{
			return componentType == ComponentType::Byte ||
				componentType == ComponentType::UnsignedByte ||
				componentType == ComponentType::Short ||
				componentType == ComponentType::UnsignedShort ||
				componentType == ComponentType::Int ||
				componentType == ComponentType::UnsignedInt;
		}

		bool IsUnsignedInteger(ComponentType componentType)
		{
			return componentType == ComponentType::UnsignedByte ||
				componentType == ComponentType::UnsignedShort ||
				componentType == ComponentType::UnsignedInt;
		}

		bool IsSignedInteger(ComponentType componentType)
		{
			return componentType == ComponentType::Byte ||
				componentType == ComponentType::Short ||
				componentType == ComponentType::Int;
		}

		bool IsFloatLike(ComponentType componentType)
		{
			return componentType == ComponentType::Float ||
				componentType == ComponentType::Double;
		}

		float ReadFloatComponent(const uint8_t* bytes, ComponentType componentType, bool normalized)
		{
			switch (componentType)
			{
			case ComponentType::Byte:
			{
				int8_t value = ReadUnaligned<int8_t>(bytes);
				return normalized ? NormalizeSigned(static_cast<float>(value), static_cast<float>(std::numeric_limits<int8_t>::max())) : static_cast<float>(value);
			}
			case ComponentType::UnsignedByte:
			{
				uint8_t value = ReadUnaligned<uint8_t>(bytes);
				return normalized ? static_cast<float>(value) / static_cast<float>(std::numeric_limits<uint8_t>::max()) : static_cast<float>(value);
			}
			case ComponentType::Short:
			{
				int16_t value = ReadUnaligned<int16_t>(bytes);
				return normalized ? NormalizeSigned(static_cast<float>(value), static_cast<float>(std::numeric_limits<int16_t>::max())) : static_cast<float>(value);
			}
			case ComponentType::UnsignedShort:
			{
				uint16_t value = ReadUnaligned<uint16_t>(bytes);
				return normalized ? static_cast<float>(value) / static_cast<float>(std::numeric_limits<uint16_t>::max()) : static_cast<float>(value);
			}
			case ComponentType::Int:
			{
				int32_t value = ReadUnaligned<int32_t>(bytes);
				return normalized ? NormalizeSigned(static_cast<float>(value), static_cast<float>(std::numeric_limits<int32_t>::max())) : static_cast<float>(value);
			}
			case ComponentType::UnsignedInt:
			{
				uint32_t value = ReadUnaligned<uint32_t>(bytes);
				return normalized
					? static_cast<float>(value) / static_cast<float>(std::numeric_limits<uint32_t>::max())
					: static_cast<float>(value);
			}
			case ComponentType::Float:
				return ReadUnaligned<float>(bytes);
			case ComponentType::Double:
				return static_cast<float>(ReadUnaligned<double>(bytes));
			}

			assert(false && "Invalid component type");
			return 0.0f;
		}

		std::expected<int64_t, Utils::Error> ReadSignedIntegerComponent(const uint8_t* bytes, ComponentType componentType)
		{
			switch (componentType)
			{
			case ComponentType::Byte:
				return static_cast<int64_t>(ReadUnaligned<int8_t>(bytes));
			case ComponentType::Short:
				return static_cast<int64_t>(ReadUnaligned<int16_t>(bytes));
			case ComponentType::Int:
				return static_cast<int64_t>(ReadUnaligned<int32_t>(bytes));
			default:
				return std::unexpected(Utils::Error("Source component type is not a signed integer"));
			}
		}

		std::expected<uint64_t, Utils::Error> ReadUnsignedIntegerComponent(const uint8_t* bytes, ComponentType componentType)
		{
			switch (componentType)
			{
			case ComponentType::UnsignedByte:
				return static_cast<uint64_t>(ReadUnaligned<uint8_t>(bytes));
			case ComponentType::UnsignedShort:
				return static_cast<uint64_t>(ReadUnaligned<uint16_t>(bytes));
			case ComponentType::UnsignedInt:
				return static_cast<uint64_t>(ReadUnaligned<uint32_t>(bytes));
			default:
				return std::unexpected(Utils::Error("Source component type is not an unsigned integer"));
			}
		}

		std::expected<void, Utils::Error> WriteFloatComponent(uint8_t* bytes, ComponentType componentType, float value)
		{
			switch (componentType)
			{
			case ComponentType::Float:
				WriteUnaligned<float>(bytes, value);
				return {};
			case ComponentType::Double:
			//??
				WriteUnaligned<double>(bytes, static_cast<double>(value));
				return {};
			default:
				return std::unexpected(Utils::Error("Destination component type cannot receive floating-point data"));
			}
		}

		std::expected<void, Utils::Error> WriteUnsignedIntegerComponent(uint8_t* bytes, ComponentType componentType, uint64_t value)
		{
			switch (componentType)
			{
			case ComponentType::UnsignedByte:
				if (value > std::numeric_limits<uint8_t>::max())
					return std::unexpected(Utils::Error("Unsigned integer conversion overflows uint8"));
				WriteUnaligned<uint8_t>(bytes, static_cast<uint8_t>(value));
				return {};
			case ComponentType::UnsignedShort:
				if (value > std::numeric_limits<uint16_t>::max())
					return std::unexpected(Utils::Error("Unsigned integer conversion overflows uint16"));
				WriteUnaligned<uint16_t>(bytes, static_cast<uint16_t>(value));
				return {};
			case ComponentType::UnsignedInt:
				if (value > std::numeric_limits<uint32_t>::max())
					return std::unexpected(Utils::Error("Unsigned integer conversion overflows uint32"));
				WriteUnaligned<uint32_t>(bytes, static_cast<uint32_t>(value));
				return {};
			default:
				return std::unexpected(Utils::Error("Destination component type cannot receive unsigned integer data"));
			}
		}

		std::expected<void, Utils::Error> WriteSignedIntegerComponent(uint8_t* bytes, ComponentType componentType, int64_t value)
		{
			switch (componentType)
			{
			case ComponentType::Byte:
				if (value < std::numeric_limits<int8_t>::min() || value > std::numeric_limits<int8_t>::max())
					return std::unexpected(Utils::Error("Signed integer conversion overflows int8"));
				WriteUnaligned<int8_t>(bytes, static_cast<int8_t>(value));
				return {};
			case ComponentType::Short:
				if (value < std::numeric_limits<int16_t>::min() || value > std::numeric_limits<int16_t>::max())
					return std::unexpected(Utils::Error("Signed integer conversion overflows int16"));
				WriteUnaligned<int16_t>(bytes, static_cast<int16_t>(value));
				return {};
			case ComponentType::Int:
				if (value < std::numeric_limits<int32_t>::min() || value > std::numeric_limits<int32_t>::max())
					return std::unexpected(Utils::Error("Signed integer conversion overflows int32"));
				WriteUnaligned<int32_t>(bytes, static_cast<int32_t>(value));
				return {};
			default:
				return std::unexpected(Utils::Error("Destination component type cannot receive signed integer data"));
			}
		}

		std::expected<void, Utils::Error> CopyComponent(
			const uint8_t* source,
			ComponentType sourceType,
			bool sourceNormalized,
			uint8_t* destination,
			ComponentType destinationType,
			bool destinationNormalized)
		{
			if (destinationNormalized)
				return std::unexpected(Utils::Error("Normalized destination accessors are not supported"));

			if (IsFloatLike(destinationType))
			{
				float value = ReadFloatComponent(source, sourceType, sourceNormalized);
				return WriteFloatComponent(destination, destinationType, value);
			}

			if (sourceNormalized)
				return std::unexpected(Utils::Error("Cannot copy normalized source data into integer destination"));

			if (IsUnsignedInteger(destinationType))
			{
				if (!IsUnsignedInteger(sourceType))
					return std::unexpected(Utils::Error("Cannot copy signed or floating-point data into unsigned integer destination"));

				CORE_TRY(value, ReadUnsignedIntegerComponent(source, sourceType));
				return WriteUnsignedIntegerComponent(destination, destinationType, value);
			}

			if (IsSignedInteger(destinationType))
			{
				if (!IsSignedInteger(sourceType))
					return std::unexpected(Utils::Error("Cannot copy unsigned or floating-point data into signed integer destination"));

				CORE_TRY(value, ReadSignedIntegerComponent(source, sourceType));
				return WriteSignedIntegerComponent(destination, destinationType, value);
			}

			return std::unexpected(Utils::Error("Unsupported destination component type"));
		}
	}


    std::expected<std::vector<uint8_t>, Utils::Error> ReadFileBytes(const std::filesystem::path& path, uint64_t byteOffset, uint64_t byteLength)
    {
        std::ifstream file(path, std::ios::binary);

        if (!file)
            return std::unexpected(Utils::Error("Failed to open buffer file '{}'", path.string()));

        file.seekg(0, std::ios::end);
        std::streamoff fileLength = file.tellg();

        if (fileLength < 0)
            return std::unexpected(Utils::Error("Failed to read buffer file length '{}'", path.string()));

        if (byteOffset + byteLength > static_cast<uint64_t>(fileLength))
        {
            return std::unexpected(Utils::Error(
                "Buffer range is outside file bounds. File: '{}', offset: {}, length: {}, file length: {}",
                path.string(),
                byteOffset,
                byteLength,
                static_cast<uint64_t>(fileLength)));
        }

        std::vector<uint8_t> bytes(static_cast<size_t>(byteLength));

        file.seekg(static_cast<std::streamoff>(byteOffset), std::ios::beg);

        if (!bytes.empty())
            file.read(reinterpret_cast<char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));

        if (!file)
            return std::unexpected(Utils::Error("Failed to read buffer file '{}'", path.string()));

        return bytes;
    }
    
    std::expected<void, Utils::Error> CopyViewData(std::span<uint8_t> sourceBuffer, const BufferView& sourceView, std::span<uint8_t> destinationBuffer, const BufferView& destinationView)
	{
		if (sourceView.count != destinationView.count)
			return std::unexpected(Utils::Error("View count mismatch. Source: {}, destination: {}", sourceView.count, destinationView.count));

		if (ElementComponentCount(sourceView.elementType) != ElementComponentCount(destinationView.elementType))
		{
			return std::unexpected(Utils::Error(
				"View element component count mismatch. Source: {}, destination: {}",
				ElementComponentCount(sourceView.elementType),
				ElementComponentCount(destinationView.elementType)));
		}

		CORE_TRY_DISCARD(ValidateView(sourceBuffer, sourceView, "Source"));
		CORE_TRY_DISCARD(ValidateView(destinationBuffer, destinationView, "Destination"));

		uint64_t sourceStride = sourceView.GetStride();
		uint64_t destinationStride = destinationView.GetStride();

		uint32_t componentCount = ElementComponentCount(sourceView.elementType);
		uint32_t sourceComponentByteLength = ComponentByteLength(sourceView.componentType);
		uint32_t destinationComponentByteLength = ComponentByteLength(destinationView.componentType);

		const uint8_t* sourceBase = sourceBuffer.data() + static_cast<size_t>(sourceView.byteOffset);
		uint8_t* destinationBase = destinationBuffer.data() + static_cast<size_t>(destinationView.byteOffset);

		for (uint32_t elementIndex = 0; elementIndex < sourceView.count; elementIndex++)
		{
			const uint8_t* sourceElement = sourceBase + static_cast<size_t>(elementIndex) * sourceStride;
			uint8_t* destinationElement = destinationBase + static_cast<size_t>(elementIndex) * destinationStride;

			for (uint32_t componentIndex = 0; componentIndex < componentCount; componentIndex++)
			{
				const uint8_t* sourceComponent = sourceElement + static_cast<size_t>(componentIndex) * sourceComponentByteLength;
				uint8_t* destinationComponent = destinationElement + static_cast<size_t>(componentIndex) * destinationComponentByteLength;

				CORE_TRY_DISCARD(CopyComponent(
					sourceComponent,
					sourceView.componentType,
					sourceView.normalized,
					destinationComponent,
					destinationView.componentType,
					destinationView.normalized));
			}
		}

		return {};
	}
}
