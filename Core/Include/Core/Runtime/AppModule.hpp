#pragma once
#include <optional>
#include <expected>
#include <Core/Runtime/Context.hpp>
#include <Core/Graphics/Gl/RenderSurface.hpp>

namespace Core::Runtime
{
	class AppModule
	{
	public:
		virtual ~AppModule() = default;
		virtual std::expected<void, Utils::Error> Configure(const ConfigureContext& context) { return {}; }
		virtual void Start(const AppContext& context) {}
		virtual void Update(const AppContext& context) {}
		virtual std::optional<Graphics::Gl::RenderSurface> GetSceneSurface() { return std::nullopt; }
		virtual void Shutdown() {}
	};
}