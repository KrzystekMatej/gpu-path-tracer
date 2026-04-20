#pragma once
#include <array>
#include <string_view>

namespace App::PathTracer
{
	enum class State
	{
		Idle,
		Active,
		Stopping,
		Finished
	};

	std::string_view ToString(State state);

	struct Status
	{
		State state = State::Idle;

		uint32_t doneFrames = 0;
		uint32_t totalFrames = 0;

		uint32_t doneSamples = 0;
		uint32_t totalSamples = 0;
	};
}