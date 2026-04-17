#include <Core/Ecs/SceneNodes/BuilderRegistry.hpp>
#include <Core/Ecs/SceneNodes/Camera.hpp>
#include <Core/Ecs/SceneNodes/Model.hpp>
#include <Core/Ecs/SceneNodes/Transform.hpp>
#include <Core/Ecs/SceneNodes/Background.hpp>
#include <Core/Ecs/SceneNodes/Light.hpp>

namespace Core::Ecs::SceneNodes
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