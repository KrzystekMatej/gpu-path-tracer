#include <Core/Capture/Resampling.hpp>
#include <Core/Utils/Math/Interpolation.hpp>

namespace Core::Capture
{
    namespace
    {
        MotionState InterpolateMotion(const MotionState& previous, const MotionState& current, float t)
        {
            MotionState result;
            result.position = Utils::Math::Interpolation::Lerp(previous.position, current.position, t);
            result.rotation = Utils::Math::Interpolation::SlerpShortest(previous.rotation, current.rotation, t);
            return result;
        }
    }

	std::vector<MotionState> ResampleMotion(const std::vector<MotionSample>& samples, float targetInterval)
	{
		assert(targetInterval > 0.0f);

		if (samples.empty())
			return {};

		if (samples.size() == 1)
			return { samples[0].state };

		const float firstTime = samples.front().time;
		const float lastTime = samples.back().time;
		const float duration = lastTime - firstTime;

		std::vector<MotionState> resampled;
		resampled.reserve(static_cast<std::size_t>(std::floor(duration / targetInterval)) + 1);
		resampled.push_back(samples[0].state);

		for (std::size_t i = 1; i < samples.size(); ++i)
		{
			const MotionSample& previous = samples[i - 1];
			const MotionSample& current = samples[i];

			assert(current.time > previous.time);

			while (true)
			{
				float targetTime = firstTime + static_cast<float>(resampled.size()) * targetInterval;

				if (targetTime > current.time)
					break;

				float t = (targetTime - previous.time) / (current.time - previous.time);
				resampled.push_back(InterpolateMotion(previous.state, current.state, t));
			}
		}

		return resampled;
	}
}