#pragma once
#include "Graphics/Material.hpp"
#include "Graphics/Gl/Resources/Texture.hpp"

namespace Core::Graphics::Gl
{
	enum class LocalShadingModel
	{
		Unlit,
		Normal,
		Lambert,
		Pbr
	};

	inline std::pair<LocalShadingModel, bool> ToLocalShadingChecked(SurfaceModel surface)
	{
		switch (surface)
		{
			case SurfaceModel::Unlit:
				return { LocalShadingModel::Unlit, true };

			case SurfaceModel::Normal:
				return { LocalShadingModel::Normal, true };

			case SurfaceModel::Diffuse:
				return { LocalShadingModel::Lambert, true };

			case SurfaceModel::Microfacet:
				return { LocalShadingModel::Pbr, true };

			case SurfaceModel::Mirror:
				return { LocalShadingModel::Unlit, false };

			case SurfaceModel::Emissive:
				return { LocalShadingModel::Unlit, false };
		}

		return { LocalShadingModel::Unlit, false };
	}

	inline LocalShadingModel ToLocalShadingUnchecked(SurfaceModel surface)
	{
		switch (surface)
		{
			case SurfaceModel::Unlit:
				return LocalShadingModel::Unlit;

			case SurfaceModel::Normal:
				return LocalShadingModel::Normal;

			case SurfaceModel::Diffuse:
				return LocalShadingModel::Lambert;

			case SurfaceModel::Microfacet:
				return LocalShadingModel::Pbr;

			case SurfaceModel::Mirror:
				return LocalShadingModel::Unlit;

			case SurfaceModel::Emissive:
				return LocalShadingModel::Unlit;
		}

		return LocalShadingModel::Unlit;
	}
	
	struct Material
	{
		LocalShadingModel localShading;
		const Texture& albedo;
		const Texture& roughness;
		const Texture& metallic;
		const Texture& ao;
		const Texture& normal;
	};
}