#pragma once
#include <Core/Capture/Sample.hpp>
#include <App/CameraRecorder/Settings.hpp>
#include <vector>

namespace App::CameraRecorder::Events
{
	struct Start
	{
		Start(const Settings& settings)
			: settings(settings) {}

		Settings settings;
	};

	struct Stop
	{
		Stop(const Settings& settings)
			: settings(settings) {}

		Settings settings;
	};

	struct Finish
	{
		Finish(std::vector<Core::Capture::MotionState> frames)
			: frames(std::move(frames)) {}

		std::vector<Core::Capture::MotionState> frames;
	};
}