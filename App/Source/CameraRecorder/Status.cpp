#include <App/CameraRecorder/Status.hpp>

namespace App::CameraRecorder
{
	constexpr std::array<std::string_view, static_cast<size_t>(State::Finished) + 1> names =
	{
		"Idle",
		"Active",
		"Finished"
	};

	std::string_view ToString(State state) {
		return names[static_cast<size_t>(state)];
	}	
}