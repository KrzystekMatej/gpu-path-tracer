#pragma once
#include <memory>
#include <Core/Graphics/Cuda/PathTracing/Renderer.hpp>
#include <Core/Ecs/Components/MotionRecorder.hpp>
#include <Core/Runtime/AppModule.hpp>
#include <Core/Events/Keyboard.hpp>
#include <Core/Graphics/Gl/RenderTarget.hpp>
#include <Core/Utils/Error.hpp>
#include <App/Controller.hpp>

namespace App::SceneViewer::Domain
{
	class AppModule : public Core::Runtime::AppModule, public App::Controller
	{
	public:
		Core::Ecs::Components::MotionRecorder::State GetCameraRecorderState() const override;
		Core::Graphics::Cuda::PathTracing::Renderer::State GetPathTracerState() const override;
		uint32_t GetRecordedFrames() const override;
		uint32_t GetTotalFrames() const override;
		uint32_t GetDoneFrames() const override;
		uint32_t GetTotalSamples() const override;
		uint32_t GetDoneSamples() const override;
		uint32_t GetSceneTextureId() const override;

		std::expected<void, Core::Utils::Error> SetRecordingSettings(uint32_t fps) override;
		std::expected<void, Core::Utils::Error> StartRecording() override;
		std::expected<void, Core::Utils::Error> StopRecording() override;
		std::expected<void, Core::Utils::Error> SetRenderingSettings(
			uint32_t frameWidth, 
			uint32_t frameHeight, 
			uint32_t samplesPerPixel, 
			uint32_t pathDepth) override;
		std::expected<void, Core::Utils::Error> StartRendering() override;
		std::expected<void, Core::Utils::Error> StopRendering() override;
		std::expected<void, Core::Utils::Error> SetSceneTargetSize(uint32_t width, uint32_t height) override;

		std::expected<void, Core::Utils::Error> Configure(const Core::Runtime::ConfigureContext& context) override;
		void Start(const Core::Runtime::AppContext& context) override;
		void Update(const Core::Runtime::AppContext& context) override;
		std::optional<Core::Graphics::Gl::RenderSurface> GetSceneSurface() override;
		void Shutdown() override;
	private:
		void OnKeyPressed(const Core::Events::KeyPressed& event);

		Core::Window::NativeWindow* m_Window = nullptr;
		std::unique_ptr<Core::Graphics::Gl::RenderTarget> m_SceneTarget;

		Core::Ecs::Components::MotionRecorder::State m_CameraRecorderState = Core::Ecs::Components::MotionRecorder::State::Idle;
		uint32_t m_RecordedFrames = 0;
		uint32_t m_TargetFps = 24;

		Core::Graphics::Cuda::PathTracing::Renderer::State m_PathTracerState = Core::Graphics::Cuda::PathTracing::Renderer::State::Idle;
		uint32_t m_DoneFrames = 0;
		uint32_t m_TotalFrames = 240;
		uint32_t m_DoneSamples = 0;
		uint32_t m_TotalSamples = 64;

		uint32_t m_FrameWidth = 1920;
		uint32_t m_FrameHeight = 1080;
		uint32_t m_SamplesPerPixel = 64;
		uint32_t m_PathDepth = 8;
	};
}
