#pragma once
#include <Core/Graphics/Common/Material.hpp>
#include <Core/Graphics/Gl/Resources/Texture.hpp>

namespace Core::Graphics::Gl
{
	enum class LocalShadingModel
	{
		Unlit,
		Normal,
		Lambert,
		Phong,
		Pbr,
		Emissive
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

			case SurfaceModel::Phong:
				return { LocalShadingModel::Phong, true };

			case SurfaceModel::Mirror:
				return { LocalShadingModel::Unlit, false };

			case SurfaceModel::Emissive:
				return { LocalShadingModel::Emissive, true };
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
			
			case SurfaceModel::Phong:
				return LocalShadingModel::Phong;

			case SurfaceModel::Microfacet:
				return LocalShadingModel::Pbr;

			case SurfaceModel::Mirror:
				return LocalShadingModel::Unlit;

			case SurfaceModel::Emissive:
				return LocalShadingModel::Emissive;
		}

		return LocalShadingModel::Unlit;
	}
	
	struct Material
	{
		LocalShadingModel localShading;
		const Texture& albedo;
		const Texture& specular;
		const Texture& shininess;
		const Texture& rma;
		const Texture& normal;
		const Texture& emission;
	};
}