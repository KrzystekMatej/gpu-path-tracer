#pragma once
#include <cstdint>
#include <expected>
#include <Core/Utils/Error.hpp>
#include <Core/Graphics/Cuda/PathTracing/Renderer.hpp>
#include <Core/Ecs/Components/MotionRecorder.hpp>

namespace App
{
	class Controller
	{
	public:
		virtual Core::Ecs::Components::MotionRecorder::State GetCameraRecorderState() const = 0;
		virtual Core::Graphics::Cuda::PathTracing::Renderer::State GetPathTracerState() const = 0;
		virtual uint32_t GetRecordedFrames() const = 0;
		virtual uint32_t GetTotalFrames() const = 0;
		virtual uint32_t GetDoneFrames() const = 0;
		virtual uint32_t GetTotalSamples() const = 0;
		virtual uint32_t GetDoneSamples() const = 0;
		virtual uint32_t GetSceneTextureId() const = 0;

		virtual std::expected<void, Core::Utils::Error> SetRecordingSettings(uint32_t fps) = 0;
		virtual std::expected<void, Core::Utils::Error> StartRecording() = 0;
		virtual std::expected<void, Core::Utils::Error> StopRecording() = 0;
		virtual std::expected<void, Core::Utils::Error> SetRenderingSettings(
			uint32_t frameWidth, 
			uint32_t frameHeight, 
			uint32_t samplesPerPixel, 
			uint32_t pathDepth) = 0;
		virtual std::expected<void, Core::Utils::Error> StartRendering() = 0;
		virtual std::expected<void, Core::Utils::Error> StopRendering() = 0;

		virtual std::expected<void, Core::Utils::Error> SetSceneTargetSize(uint32_t width, uint32_t height) = 0;
	};
}