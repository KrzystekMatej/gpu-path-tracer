#pragma once
#include <array>
#include <string_view>

namespace App::CameraRecorder
{
	enum class State
	{
		Idle,
		Active,
		Finished
	};

	std::string_view ToString(State state);

	struct Status
	{
		State state = State::Idle;

		uint32_t doneFrames = 0;
	};
}