#pragma once
#include <array>
#include <string_view>
#include <Core/Graphics/Cuda/PathTracing/Renderer.hpp>

namespace App::PathTracer
{
	enum class State
	{
		Idle,
		Ready,
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
		Core::Graphics::Cuda::Renderer::FrameView currentFrame;
	};
}