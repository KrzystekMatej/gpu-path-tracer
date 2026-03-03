#pragma once
#include "Graphics/ShadingModel.hpp"

namespace Core::Graphics
{
struct MaterialDefaults
	{
		static constexpr ShadingModel DefaultShader = ShadingModel::Lambert;

		static constexpr std::array<uint8_t, 3> DefaultAlbedo = { 255, 255, 255 };
		static constexpr std::array<uint8_t, 1> DefaultRoughness = { 255 };
		static constexpr std::array<uint8_t, 1> DefaultMetallic = { 0 };

		static constexpr std::array<uint8_t, 1> DefaultAo = { 255 };
		static constexpr std::array<uint8_t, 3> DefaultNormal = { 128, 128, 255 };
	};
}