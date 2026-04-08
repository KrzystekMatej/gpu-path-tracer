#pragma once
#include <array>

namespace Core::Graphics::Common
{
	enum class SurfaceModel
	{
		Unlit,
		Normal,
		Diffuse,
		Microfacet,
		Mirror,
		Emissive,
	};

	enum class GlobalShadingModel
	{
		Unlit,
		Normal,
		Diffuse,
		Mirror,
		Ggx,
		Emissive,
	};

	struct MaterialDefaults
	{
		static constexpr SurfaceModel DefaultSurfaceModel = SurfaceModel::Unlit;

		static constexpr std::array<uint8_t, 3> DefaultAlbedo = { 255, 255, 255 };
		static constexpr std::array<uint8_t, 1> DefaultRoughness = { 255 };
		static constexpr std::array<uint8_t, 1> DefaultMetallic = { 0 };

		static constexpr std::array<uint8_t, 1> DefaultAo = { 255 };
		static constexpr std::array<uint8_t, 3> DefaultNormal = { 128, 128, 255 };
	};
}