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
		assert(Register("Transform", std::make_unique<TransformBuilder>()));
		assert(Register("Camera", std::make_unique<Graphics::Ecs::CameraBuilder>()));
		assert(Register("Model", std::make_unique<Graphics::Ecs::ModelBuilder>()));
		assert(Register("Background", std::make_unique<Graphics::Ecs::BackgroundBuilder>()));
		assert(Register("Light", std::make_unique<Graphics::Ecs::LightBuilder>()));
		assert(Register("MotionRecorder", std::make_unique<Capture::MotionRecorderBuilder>()));
		assert(Register("Grid", std::make_unique<Graphics::Ecs::GridBuilder>()));
	}
}