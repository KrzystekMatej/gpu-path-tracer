#include <App/SceneViewer/Domain/AppModule.hpp>
#include <App/Scripts/CameraController.hpp>

namespace App::SceneViewer::Domain
{
	Core::Ecs::Components::MotionRecorder::State AppModule::GetCameraRecorderState() const { return m_CameraRecorderState; }
	Core::Graphics::Cuda::PathTracing::Renderer::State AppModule::GetPathTracerState() const { return m_PathTracerState; }
	uint32_t AppModule::GetRecordedFrames() const { return m_RecordedFrames; }
	uint32_t AppModule::GetTotalFrames() const { return m_TotalFrames; }
	uint32_t AppModule::GetDoneFrames() const { return m_DoneFrames; }
	uint32_t AppModule::GetTotalSamples() const { return m_TotalSamples; }
	uint32_t AppModule::GetDoneSamples() const { return m_DoneSamples; }
	uint32_t AppModule::GetSceneTextureId() const { return m_SceneTarget->GetTexture().GetId(); }

	std::expected<void, Core::Utils::Error> AppModule::SetRecordingSettings(uint32_t fps)
	{ 
		m_TargetFps = fps;
		return {}; 
	}
	std::expected<void, Core::Utils::Error> AppModule::StartRecording() { return {}; }
	std::expected<void, Core::Utils::Error> AppModule::StopRecording() { return {}; }

	std::expected<void, Core::Utils::Error> AppModule::SetRenderingSettings(
		uint32_t frameWidth, 
		uint32_t frameHeight, 
		uint32_t samplesPerPixel, 
		uint32_t pathDepth)
	{
		m_FrameWidth = frameWidth;
		m_FrameHeight = frameHeight;
		m_SamplesPerPixel = samplesPerPixel;
		m_PathDepth = pathDepth;
		return {}; 
	}
	std::expected<void, Core::Utils::Error> AppModule::StartRendering() { return {}; }
	std::expected<void, Core::Utils::Error> AppModule::StopRendering() { return {}; }
	std::expected<void, Core::Utils::Error> AppModule::SetSceneTargetSize(uint32_t width, uint32_t height)
	{
		if (!m_SceneTarget)
			return std::unexpected(Core::Utils::Error("Scene render target is not initialized"));

		if (width <= 0 || height <= 0)
			return std::unexpected(Core::Utils::Error("Scene render target size must be greater than 0"));

		if (width != m_SceneTarget->GetWidth() || height != m_SceneTarget->GetHeight())
			m_SceneTarget->Resize(width, height);

		return {}; 
	}

	std::expected<void, Core::Utils::Error> AppModule::Configure(const Core::Runtime::ConfigureContext& context)
	{
		context.scriptCatalog.Register("AwakeCameraController", Scripts::AwakeCameraController);
		context.scriptCatalog.Register("UpdateCameraController", Scripts::UpdateCameraController);

		context.builderRegistry.Register("CameraController", std::make_unique<Scripts::CameraControllerBuilder>());

		m_Window = &context.window;
		context.eventDispatcher.sink<Core::Events::KeyPressed>().connect<&AppModule::OnKeyPressed>(*this);

		m_SceneTarget = std::make_unique<Core::Graphics::Gl::RenderTarget>(context.window.GetWidth(), context.window.GetHeight());
		return {};
	}

	void AppModule::OnKeyPressed(const Core::Events::KeyPressed& event)
	{
		if (!event.repeat && event.key == Core::Input::KeyCode::Escape)
			m_Window->Close();
	}

	void AppModule::Start(const Core::Runtime::AppContext& context)
	{
	}

	void AppModule::Update(const Core::Runtime::AppContext& context)
	{
	}

	std::optional<Core::Graphics::Gl::RenderSurface> AppModule::GetSceneSurface()
	{
		if (m_SceneTarget)
			return m_SceneTarget->GetRenderSurface();
		return std::nullopt;
	}

	void AppModule::Shutdown()
	{
	}
}
