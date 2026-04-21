#pragma once
#include <Core/Assets/Types.hpp>
#include <Core/Ecs/Builder.hpp>

namespace Core::Graphics::Ecs
{
    struct Mesh
    {
        explicit Mesh(Assets::Handle<Assets::Mesh> handle) : handle(handle) {}

		Assets::Handle<Assets::Mesh> handle;
    };

    struct Material
    {
        explicit Material(Assets::Handle<Assets::Material> handle) : handle(handle) {}

		Assets::Handle<Assets::Material> handle;
    };

	class ModelBuilder : public Core::Ecs::Builder
	{
	public:
		virtual std::expected<void, Utils::Error> Build(
			const Core::Ecs::BuildContext& context,
			entt::registry& registry,
			Assets::Manager& assetManager) const override;
	};

    struct Background
    {
		explicit Background(Assets::Handle<Assets::EnvironmentMap> handle) : handle(handle) {}

		Assets::Handle<Assets::EnvironmentMap> handle;
    };

	class BackgroundBuilder : public Core::Ecs::Builder
	{
	public:
		std::expected<void, Utils::Error> Build(
			const Core::Ecs::BuildContext& context,
			entt::registry& registry,
			Assets::Manager& assetManager) const override;
	};
}