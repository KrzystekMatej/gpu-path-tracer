#include <Core/Capture/Recording.hpp>

namespace Core::Capture
{
	void StartMotionRecording(ECS::Scene& scene, entt::entity entity, float sampleInterval)
	{
		if (auto* recorder = scene.GetRegistry().try_get<ECS::Components::MotionRecorder>(entity))
		{
			recorder->state = ECS::Components::MotionRecorderState::Start;
			recorder->sampleInterval = sampleInterval;
			return;
		}

		scene.GetRegistry().emplace<ECS::Components::MotionRecorder>(entity, sampleInterval, ECS::Components::MotionRecorderState::Start);
	}

	std::expected<std::reference_wrapper<const std::vector<MotionSample>>, Utils::Error> StopMotionRecording(
		ECS::Scene& scene,
		entt::entity entity)
	{
		if (auto* recorder = scene.GetRegistry().try_get<ECS::Components::MotionRecorder>(entity))
		{
			recorder->state = ECS::Components::MotionRecorderState::Finished;
			return recorder->GetSamples();
		}
		return std::unexpected(Utils::Error("Entity does not have a MotionRecorder component"));
	}

	std::expected<void, Utils::Error> ClearMotionRecording(ECS::Scene& scene, entt::entity entity)
	{
		if (auto* recorder = scene.GetRegistry().try_get<ECS::Components::MotionRecorder>(entity))
		{
			recorder->ClearSamples();
			recorder->state = ECS::Components::MotionRecorderState::Idle;
			return {};
		}
		return std::unexpected(Utils::Error("Entity does not have a MotionRecorder component"));
	}
}