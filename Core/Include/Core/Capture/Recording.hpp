#pragma once
#include <expected>
#include <functional>
#include <Core/Ecs/Components/MotionRecorder.hpp>
#include <Core/Ecs/Scene.hpp>

namespace Core::Capture
{
	void StartMotionRecording(Ecs::Scene& scene, entt::entity entity, float sampleInterval);
	std::expected<std::reference_wrapper<const std::vector<MotionSample>>, Utils::Error> StopMotionRecording(Ecs::Scene& scene, entt::entity entity);
	std::expected<void, Utils::Error> ClearMotionRecording(Ecs::Scene& scene, entt::entity entity);
}