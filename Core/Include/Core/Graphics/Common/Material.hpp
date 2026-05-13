#pragma once
#include <array>

namespace Core::Graphics
{
	enum class SurfaceModel
	{
		Unlit,
		Normal,
		Diffuse,
		Phong,
		Microfacet,
		Mirror,
		Emissive,
	};

	struct MaterialDefaults
	{
		static constexpr SurfaceModel DefaultSurfaceModel = SurfaceModel::Unlit;

		static constexpr std::array<uint8_t, 3> DefaultAlbedo = { 255, 255, 255 };

		static constexpr std::array<uint8_t, 3> DefaultPhongSpecular = { 0, 0, 0 };
		static constexpr std::array<uint8_t, 3> DefaultMirrorReflectance = { 255, 255, 255 };
		static constexpr float MinShininess = 1.0f;
		static constexpr float MaxShininess = 1000.0f;
		static constexpr std::array<uint8_t, 1> DefaultShininess = { 0 };	

		static constexpr std::array<uint8_t, 1> DefaultRoughness = { 255 };
		static constexpr std::array<uint8_t, 1> DefaultMetallic = { 0 };
		static constexpr std::array<uint8_t, 1> DefaultAo = { 255 };

		static constexpr std::array<float, 3> DefaultEmission = { 0, 0, 0 };
		
		static constexpr std::array<uint8_t, 3> DefaultNormal = { 128, 128, 255 };
	};
}