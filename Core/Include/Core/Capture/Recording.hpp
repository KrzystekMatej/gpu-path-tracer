#pragma once
#include <expected>
#include <functional>
#include <Core/ECS/Components/MotionRecorder.hpp>
#include <Core/ECS/Scene.hpp>

namespace Core::Capture
{
	void StartMotionRecording(ECS::Scene& scene, entt::entity entity, float sampleInterval);
	std::expected<std::reference_wrapper<const std::vector<MotionSample>>, Utils::Error> StopMotionRecording(ECS::Scene& scene, entt::entity entity);
	std::expected<void, Utils::Error> ClearMotionRecording(ECS::Scene& scene, entt::entity entity);
}