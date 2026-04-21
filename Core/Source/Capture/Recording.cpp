#include <Core/Capture/Recording.hpp>

namespace Core::Capture
{
	void InitMotionRecording(Ecs::Scene& scene, entt::entity entity, float sampleInterval)
	{
		if (auto* recorder = scene.GetRegistry().try_get<MotionRecorder>(entity))
		{
			recorder->state = MotionRecorder::State::Idle;
			recorder->sampleInterval = sampleInterval;
			return;
		}
		scene.GetRegistry().emplace<MotionRecorder>(entity, sampleInterval, MotionRecorder::State::Idle);
	}

	MotionRecorder& GetMotionRecorder(Ecs::Scene& scene, entt::entity entity)
	{
		return scene.GetRegistry().get<MotionRecorder>(entity);
	}

	std::expected<std::reference_wrapper<MotionRecorder>, Utils::Error> TryGetMotionRecorder(Ecs::Scene& scene, entt::entity entity)
	{
		if (auto* recorder = scene.GetRegistry().try_get<MotionRecorder>(entity))
		{
			return std::ref(*recorder);
		}
		return std::unexpected(Utils::Error("Entity does not have a MotionRecorder component"));
	}

	std::expected<void, Utils::Error> StartMotionRecording(Ecs::Scene& scene, entt::entity entity, float sampleInterval)
	{
		if (auto* recorder = scene.GetRegistry().try_get<MotionRecorder>(entity))
		{
			if (recorder->state == MotionRecorder::State::Active)
				return std::unexpected(Utils::Error("Motion recording is already active for this entity!"));
			recorder->state = MotionRecorder::State::Start;
			recorder->sampleInterval = sampleInterval;
			return {};
		}

		scene.GetRegistry().emplace<MotionRecorder>(entity, sampleInterval, MotionRecorder::State::Start);
		return {};
	}

	std::expected<std::reference_wrapper<const std::vector<MotionSample>>, Utils::Error> StopMotionRecording(
		Ecs::Scene& scene,
		entt::entity entity)
	{
		if (auto* recorder = scene.GetRegistry().try_get<MotionRecorder>(entity))
		{
			if (recorder->state != MotionRecorder::State::Active)
				return std::unexpected(Utils::Error("Only active motion recordings can be stopped!"));
			recorder->state = MotionRecorder::State::Finished;
			return recorder->GetSamples();
		}
		return std::unexpected(Utils::Error("Entity does not have a MotionRecorder component"));
	}
}