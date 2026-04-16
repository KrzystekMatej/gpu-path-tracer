#include <Core/ECS/Systems/Capture.hpp>
#include <Core/ECS/Components/MotionRecorder.hpp>
#include <Core/ECS/Components/Transform.hpp>
#include <Core/Utils/Math/Transform.hpp>
#include <Core/Utils/Math/Interpolation.hpp>

namespace Core::ECS::Systems
{
	void UpdateMotionRecording(Scene& scene, float deltaTime)
	{
		auto view = scene.GetRegistry().view<Components::MotionRecorder, Components::WorldTransform>();
		for (auto [entity, recorder, transform] : view.each())
		{
			if (recorder.state == Components::MotionRecorderState::Start)
			{
				recorder.samples.clear();
				recorder.state = Components::MotionRecorderState::Active;
				recorder.elapsedTime = 0.0f;
				recorder.nextSampleTime = recorder.sampleInterval;

				const glm::mat4& worldMatrix = transform.GetMatrix();
				Capture::MotionState initialState{
					Core::Utils::Math::ExtractTranslation(worldMatrix),
					Core::Utils::Math::ExtractRotation(worldMatrix)
				};

				recorder.samples.push_back({
					initialState,
					0.0f
				});
				continue;
			}

			if (recorder.state != Components::MotionRecorderState::Active)
				continue;

			recorder.elapsedTime += deltaTime;

			if (recorder.elapsedTime >= recorder.nextSampleTime)
			{
				const glm::mat4& worldMatrix = transform.GetMatrix();
				Capture::MotionState state{
					Core::Utils::Math::ExtractTranslation(worldMatrix),
					Core::Utils::Math::ExtractRotation(worldMatrix)
				};

				recorder.samples.push_back({
					state,
					recorder.elapsedTime
				});

				int nextIndex = static_cast<int>(recorder.elapsedTime / recorder.sampleInterval) + 1;
				recorder.nextSampleTime = nextIndex * recorder.sampleInterval;
			}
		}
	}
}