#pragma once
#include <Core/Ecs/Builder.hpp>
#include <Core/Ecs/Scene.hpp>
#include <Core/Capture/Sample.hpp>
#include <vector>

namespace Core::Capture
{
	struct MotionRecorder
	{
	public:
		enum class State
		{
			Idle,
			Start,
			Active,
			Finished,
		};

		MotionRecorder() = default;
		MotionRecorder(float sampleInterval)
			: sampleInterval(sampleInterval) {}
		MotionRecorder(float sampleInterval, State state)
			: sampleInterval(sampleInterval), state(state) {}

		const std::vector<Capture::MotionSample>& GetSamples() const { return samples; }
		void ClearSamples() { samples.clear(); }

		float sampleInterval = 1.0f / 60.0f;
		State state = State::Idle;
	private:
		friend void UpdateMotionRecording(Ecs::Scene& scene, float deltaTime);

		float elapsedTime = 0.0f;
		float nextSampleTime = 0.0f;
		std::vector<Capture::MotionSample> samples;
	};

	class MotionRecorderBuilder : public Ecs::Builder
	{
	public:
		virtual std::expected<void, Utils::Error> Build(
			const Ecs::BuildContext& context,
			entt::registry& registry,
			Assets::Manager& assetManager) const override;
	};

	void UpdateMotionRecording(Ecs::Scene& scene, float deltaTime);
}