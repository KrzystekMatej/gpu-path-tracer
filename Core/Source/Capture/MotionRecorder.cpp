#include <Core/Capture/MotionRecorder.hpp>
#include <Core/Utils/Yaml.hpp>
#include <Core/Ecs/Transform.hpp>
#include <Core/Utils/Math/Transform.hpp>
#include <Core/Utils/Math/Interpolation.hpp>
#include <Core/Utils/Math/Glm.hpp>

namespace Core::Capture
{
	std::expected<void, Utils::Error> MotionRecorderBuilder::Build(
		const Ecs::BuildContext& context,
		entt::registry& registry,
		Assets::Manager& assetManager) const
	{
		registry.emplace<MotionRecorder>(context.entity);
		return {};
	}

	void UpdateMotionRecording(Ecs::Scene& scene, float deltaTime)
	{
		auto view = scene.GetRegistry().view<MotionRecorder, Ecs::WorldTransform>();
		for (auto [entity, recorder, transform] : view.each())
		{
			if (recorder.state == MotionRecorder::State::Start)
			{
				recorder.samples.clear();
				recorder.state = MotionRecorder::State::Active;
				recorder.elapsedTime = 0.0f;
				recorder.nextSampleTime = recorder.sampleInterval;

				const glm::mat4& worldMatrix = transform.GetMatrix();
				Capture::MotionState initialState{
					Core::Utils::Math::ExtractTranslation(worldMatrix),
					Core::Utils::Math::ExtractRotationUniformScale(worldMatrix)
				};

				recorder.samples.push_back({
					initialState,
					0.0f
				});
				continue;
			}

			if (recorder.state != MotionRecorder::State::Active)
				continue;

			recorder.elapsedTime += deltaTime;

			if (recorder.elapsedTime >= recorder.nextSampleTime)
			{
				const glm::mat4& worldMatrix = transform.GetMatrix();
				Capture::MotionState state{
					Core::Utils::Math::ExtractTranslation(worldMatrix),
					Core::Utils::Math::ExtractRotationUniformScale(worldMatrix)
				};

				recorder.samples.push_back({
					state,
					recorder.elapsedTime
				});

				int nextIndex = static_cast<int>(recorder.elapsedTime / recorder.sampleInterval) + 1;
				recorder.nextSampleTime = nextIndex * recorder.sampleInterval;

				spdlog::info("Recording motion sample at time {:.2f}s (position: {}, quaternion: {}, euler: {})", 
					recorder.elapsedTime, 
					Utils::Math::ToString(state.position), 
					Utils::Math::ToString(state.rotation),
					Utils::Math::ToStringEulerRadians(state.rotation));
			}
		}
	}
}