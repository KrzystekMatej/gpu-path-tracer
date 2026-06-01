#include <Core/Ecs/BuilderRegistry.hpp>
#include <Core/Graphics/Ecs/Camera.hpp>
#include <Core/Graphics/Ecs/Light.hpp>
#include <Core/Graphics/Ecs/Assets.hpp>
#include <Core/Ecs/Transform.hpp>
#include <Core/Capture/MotionRecorder.hpp>

namespace Core::Ecs
{
	std::expected<void, Utils::Error> BuilderRegistry::RegisterCoreBuilders()
	{
		CORE_TRY_DISCARD_CONTEXT(Register("Transform", std::make_unique<TransformBuilder>()), "Failed to register TransformBuilder");
		CORE_TRY_DISCARD_CONTEXT(Register("Camera", std::make_unique<Graphics::Ecs::CameraBuilder>()), "Failed to register CameraBuilder");
		CORE_TRY_DISCARD_CONTEXT(Register("Model", std::make_unique<Graphics::Ecs::ModelBuilder>()), "Failed to register ModelBuilder");
		CORE_TRY_DISCARD_CONTEXT(Register("Background", std::make_unique<Graphics::Ecs::BackgroundBuilder>()), "Failed to register BackgroundBuilder");
		CORE_TRY_DISCARD_CONTEXT(Register("Light", std::make_unique<Graphics::Ecs::LightBuilder>()), "Failed to register LightBuilder");
		CORE_TRY_DISCARD_CONTEXT(Register("MotionRecorder", std::make_unique<Capture::MotionRecorderBuilder>()), "Failed to register MotionRecorderBuilder");
		CORE_TRY_DISCARD_CONTEXT(Register("Grid", std::make_unique<Graphics::Ecs::GridBuilder>()), "Failed to register GridBuilder");
		return {};
	}
}