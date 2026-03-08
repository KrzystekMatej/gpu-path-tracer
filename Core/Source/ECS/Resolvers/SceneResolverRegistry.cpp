#include "ECS/Resolvers/SceneResolverRegistry.hpp"
#include "ECS/Resolvers/Camera.hpp"
#include "ECS/Resolvers/Model.hpp"
#include "ECS/Resolvers/Transform.hpp"

namespace Core::ECS
{
	void SceneResolverRegistry::RegisterCoreResolvers()
	{
		RegisterResolver("Transform", std::make_unique<TransformResolver>());
		RegisterResolver("Camera", std::make_unique<CameraResolver>());
		RegisterResolver("Model", std::make_unique<ModelResolver>());
	}
}