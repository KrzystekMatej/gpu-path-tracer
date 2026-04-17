#include <Core/Capture/Recording.hpp>

namespace Core::Capture
{
	using MotionRecorder = Ecs::Components::MotionRecorder;

	void StartMotionRecording(Ecs::Scene& scene, entt::entity entity, float sampleInterval)
	{
		if (auto* recorder = scene.GetRegistry().try_get<MotionRecorder>(entity))
		{
			recorder->state = MotionRecorder::State::Start;
			recorder->sampleInterval = sampleInterval;
			return;
		}

		scene.GetRegistry().emplace<MotionRecorder>(entity, sampleInterval, MotionRecorder::State::Start);
	}

	std::expected<std::reference_wrapper<const std::vector<MotionSample>>, Utils::Error> StopMotionRecording(
		Ecs::Scene& scene,
		entt::entity entity)
	{
		if (auto* recorder = scene.GetRegistry().try_get<MotionRecorder>(entity))
		{
			recorder->state = MotionRecorder::State::Finished;
			return recorder->GetSamples();
		}
		return std::unexpected(Utils::Error("Entity does not have a MotionRecorder component"));
	}

	std::expected<void, Utils::Error> ClearMotionRecording(Ecs::Scene& scene, entt::entity entity)
	{
		if (auto* recorder = scene.GetRegistry().try_get<MotionRecorder>(entity))
		{
			recorder->ClearSamples();
			recorder->state = MotionRecorder::State::Idle;
			return {};
		}
		return std::unexpected(Utils::Error("Entity does not have a MotionRecorder component"));
	}
}