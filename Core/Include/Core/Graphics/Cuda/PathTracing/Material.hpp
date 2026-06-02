#pragma once
#include <cuda_runtime.h>
#include <Core/Graphics/Common/Material.hpp>
#include <Core/Graphics/Cuda/Resources/TextureView.hpp>

namespace Core::Graphics::Cuda
{
	enum class GlobalShadingModel
	{
		// Unlit,
		Normal,
		Diffuse,
		Phong,
		Ggx,
		Mirror,
		Emissive,
		Count
	};


	inline std::pair<GlobalShadingModel, bool> ToGlobalShadingChecked(SurfaceModel surface)
	{
		switch (surface)
		{
			case SurfaceModel::Normal:
				return { GlobalShadingModel::Normal, true };
				
			case SurfaceModel::Diffuse:
				return { GlobalShadingModel::Diffuse, true };

			case SurfaceModel::Phong:
				return { GlobalShadingModel::Phong, true };

			case SurfaceModel::Microfacet:
				return { GlobalShadingModel::Ggx, true };

			case SurfaceModel::Mirror:
				return { GlobalShadingModel::Mirror, true };

			case SurfaceModel::Emissive:
				return { GlobalShadingModel::Emissive, true };
		}

		return { GlobalShadingModel::Diffuse, false };
	}

	inline GlobalShadingModel ToGlobalShadingUnchecked(SurfaceModel surface)
	{
		switch (surface)
		{
			case SurfaceModel::Normal:
				return GlobalShadingModel::Normal;

			case SurfaceModel::Diffuse:
				return GlobalShadingModel::Diffuse;

			case SurfaceModel::Phong:
				return GlobalShadingModel::Phong;

			case SurfaceModel::Microfacet:
				return GlobalShadingModel::Ggx;

			case SurfaceModel::Mirror:
				return GlobalShadingModel::Mirror;

			case SurfaceModel::Emissive:
				return GlobalShadingModel::Emissive;
		}

		return GlobalShadingModel::Diffuse;
	}

	struct Material
	{
		GlobalShadingModel shadingModel;
		TextureView<float4> color;
		TextureView<float4> specular;
		TextureView<float> shininess;
		TextureView<float4> rma;
		TextureView<float4> emission;
		TextureView<float4> normal;
		float ior;
		float transmission;
	};
}