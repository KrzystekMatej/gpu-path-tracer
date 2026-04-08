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
		Pbr
	};

	inline std::pair<LocalShadingModel, bool> ToLocalShadingChecked(Common::SurfaceModel surface)
	{
		switch (surface)
		{
			case Common::SurfaceModel::Unlit:
				return { LocalShadingModel::Unlit, true };

			case Common::SurfaceModel::Normal:
				return { LocalShadingModel::Normal, true };

			case Common::SurfaceModel::Diffuse:
				return { LocalShadingModel::Lambert, true };

			case Common::SurfaceModel::Microfacet:
				return { LocalShadingModel::Pbr, true };

			case Common::SurfaceModel::Mirror:
				return { LocalShadingModel::Unlit, false };

			case Common::SurfaceModel::Emissive:
				return { LocalShadingModel::Unlit, false };
		}

		return { LocalShadingModel::Unlit, false };
	}

	inline LocalShadingModel ToLocalShadingUnchecked(Common::SurfaceModel surface)
	{
		switch (surface)
		{
			case Common::SurfaceModel::Unlit:
				return LocalShadingModel::Unlit;

			case Common::SurfaceModel::Normal:
				return LocalShadingModel::Normal;

			case Common::SurfaceModel::Diffuse:
				return LocalShadingModel::Lambert;

			case Common::SurfaceModel::Microfacet:
				return LocalShadingModel::Pbr;

			case Common::SurfaceModel::Mirror:
				return LocalShadingModel::Unlit;

			case Common::SurfaceModel::Emissive:
				return LocalShadingModel::Unlit;
		}

		return LocalShadingModel::Unlit;
	}
	
	struct Material
	{
		LocalShadingModel localShading;
		const Resources::Texture& albedo;
		const Resources::Texture& roughness;
		const Resources::Texture& metallic;
		const Resources::Texture& ao;
		const Resources::Texture& normal;
	};
}