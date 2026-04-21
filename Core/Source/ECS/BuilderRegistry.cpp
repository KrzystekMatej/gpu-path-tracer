#include <Core/Ecs/BuilderRegistry.hpp>
#include <Core/Graphics/Ecs/Camera.hpp>
#include <Core/Graphics/Ecs/Light.hpp>
#include <Core/Graphics/Ecs/Assets.hpp>
#include <Core/Ecs/Transform.hpp>
#include <Core/Capture/MotionRecorder.hpp>

namespace Core::Ecs
{
	void BuilderRegistry::RegisterCoreBuilders()
	{
		Register("Transform", std::make_unique<TransformBuilder>());
		Register("Camera", std::make_unique<Graphics::Ecs::CameraBuilder>());
		Register("Model", std::make_unique<Graphics::Ecs::ModelBuilder>());
		Register("Background", std::make_unique<Graphics::Ecs::BackgroundBuilder>());
		Register("Light", std::make_unique<Graphics::Ecs::LightBuilder>());
		Register("MotionRecorder", std::make_unique<Capture::MotionRecorderBuilder>());
	}
}