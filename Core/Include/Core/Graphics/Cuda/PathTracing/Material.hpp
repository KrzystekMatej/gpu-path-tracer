#pragma once
#include <Core/Graphics/Common/Material.hpp>
#include <Core/Graphics/Cuda/Resources/TextureView.hpp>

namespace Core::Graphics::Cuda
{
	enum class GlobalShadingModel
	{
		Unlit,
		Normal,
		Diffuse,
		Ggx,
		Mirror,
		Emissive,
	};

	inline GlobalShadingModel ToGlobalShadingUnchecked(SurfaceModel surface)
	{
		switch (surface)
		{
			case SurfaceModel::Unlit:
				return GlobalShadingModel::Unlit;

			case SurfaceModel::Normal:
				return GlobalShadingModel::Normal;

			case SurfaceModel::Diffuse:
				return GlobalShadingModel::Diffuse;

			case SurfaceModel::Microfacet:
				return GlobalShadingModel::Ggx;

			case SurfaceModel::Mirror:
				return GlobalShadingModel::Mirror;

			case SurfaceModel::Emissive:
				return GlobalShadingModel::Emissive;
		}

		return GlobalShadingModel::Unlit;
	}

	struct Material
	{
		GlobalShadingModel shadingModel;
		TextureView<float> albedo;
		TextureView<float> roughness;
		TextureView<float> metallic;
		TextureView<float> ao;
		TextureView<float> normal;
	};
}