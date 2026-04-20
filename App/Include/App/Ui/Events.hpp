#pragma once

namespace App::Ui::Events
{
	struct CameraRecordingToggled
	{
		CameraRecordingToggled(bool enabled) : enabled(enabled) {}

		bool enabled = false;
	};

	struct PathTracingToggled
	{
		PathTracingToggled(bool enabled) : enabled(enabled) {}

		bool enabled = false;
	};
}