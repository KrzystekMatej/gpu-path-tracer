#pragma once
#include <App/PathTracer/Settings.hpp>

namespace App::PathTracer::Events
{
	struct Start
	{
		Start(Settings settings) 
			: settings(std::move(settings)) {}

		Settings settings;
	};

	struct Stop
	{
		Stop() = default;
	};
}