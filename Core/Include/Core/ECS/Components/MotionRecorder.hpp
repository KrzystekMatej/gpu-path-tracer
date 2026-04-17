#pragma once
#include <Core/Capture/Types.hpp>
#include <vector>

namespace Core::Ecs
{
    class Scene;

    namespace Systems
    {
		void UpdateMotionRecording(Scene& scene, float deltaTime);
    }
}

namespace Core::Ecs::Components
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
		friend void Systems::UpdateMotionRecording(Scene& scene, float deltaTime);

		float elapsedTime = 0.0f;
		float nextSampleTime = 0.0f;
		std::vector<Capture::MotionSample> samples;
	};
}