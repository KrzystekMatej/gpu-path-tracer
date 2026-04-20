#include <App/PathTracer/Status.hpp>

namespace App::PathTracer
{
	constexpr std::array<std::string_view, static_cast<size_t>(State::Finished) + 1> names =
	{
		"Idle",
		"Active",
		"Stopping",
		"Finished"
	};

	std::string_view ToString(State state) {
		return names[static_cast<size_t>(state)];
	}	
}