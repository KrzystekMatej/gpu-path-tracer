#pragma once
#include <expected>
#include <filesystem>
#include <span>
#include <Core/Import/Buffers/Buffer.hpp>
#include <Core/Utils/Error.hpp>

namespace Core::Import
{
    std::expected<std::vector<uint8_t>, Utils::Error> ReadFileBytes(const std::filesystem::path& path, uint64_t byteOffset, uint64_t byteLength);
    std::expected<void, Utils::Error> CopyViewData(std::span<uint8_t> destinationBuffer, const BufferView& destinationView, std::span<const uint8_t> sourceBuffer, const BufferView& sourceView);
}