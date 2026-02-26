#pragma once
#include "Graphics/ShadingModel.hpp"

namespace Core::Graphics
{
	struct MaterialDefaults
	{
		static constexpr ShadingModel DefaultShader = ShadingModel::Lambert;
	};
}