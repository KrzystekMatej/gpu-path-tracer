#include <Core/ECS/SceneNodes/BuilderRegistry.hpp>
#include <Core/ECS/SceneNodes/Camera.hpp>
#include <Core/ECS/SceneNodes/Model.hpp>
#include <Core/ECS/SceneNodes/Transform.hpp>
#include <Core/ECS/SceneNodes/Background.hpp>
#include <Core/ECS/SceneNodes/Light.hpp>

namespace Core::ECS::SceneNodes
{
	void BuilderRegistry::RegisterCoreBuilders()
	{
		Register("Transform", std::make_unique<TransformBuilder>());
		Register("Camera", std::make_unique<CameraBuilder>());
		Register("Model", std::make_unique<ModelBuilder>());
		Register("Background", std::make_unique<BackgroundBuilder>());
		Register("Light", std::make_unique<LightBuilder>());
	}
}