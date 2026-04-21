#pragma once
#include <expected>
#include <functional>
#include <Core/Capture/MotionRecorder.hpp>
#include <Core/Ecs/Scene.hpp>

namespace Core::Capture
{
	void InitMotionRecording(Ecs::Scene& scene, entt::entity entity, float sampleInterval);

	std::expected<std::reference_wrapper<MotionRecorder>, Utils::Error> TryGetMotionRecorder(Ecs::Scene& scene, entt::entity entity);
	MotionRecorder& GetMotionRecorder(Ecs::Scene& scene, entt::entity entity);

	std::expected<void, Utils::Error> StartMotionRecording(Ecs::Scene& scene, entt::entity entity, float sampleInterval);
	std::expected<std::reference_wrapper<const std::vector<MotionSample>>, Utils::Error> StopMotionRecording(Ecs::Scene& scene, entt::entity entity);
}