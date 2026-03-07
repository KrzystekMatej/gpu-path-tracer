#pragma once
#include <entt/entt.hpp>
#include <expected>
#include "Utils/Error/Error.hpp"
#include "Scripts/Binding.hpp"
#include "Scripts/Phase.hpp"
#include "ECS/Resolvers/SceneResolverRegistry.hpp"
#include "IO/Scene.hpp"


namespace Core::ECS
{
    enum class SceneState
    {
        Empty,
        Ready,
        Modified
    };

    class Scene
    {
    public:
		Scene() = default;
        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;
        Scene(Scene&&) = default;
        Scene& operator=(Scene&&) = default;

		static std::expected<Scene, Utils::Error> Create(IO::Scene scene, const ECS::SceneResolverRegistry& resolverRegistry);

        entt::registry& GetRegistry() { return m_Registry; }
        const entt::registry& GetRegistry() const { return m_Registry; }
        entt::entity GetActiveCamera() const { return  m_Camera; }

        SceneState GetState() const { return m_State; }
        void SetState(SceneState state) { m_State = state; }

		const std::vector<Scripts::Binding>& GetScriptBindings() const { return m_ScriptBindings; }
    private:
        Scene(
            entt::registry registry, 
            entt::entity camera, 
            SceneState state, 
            std::vector<Scripts::Binding> scriptBindings)
            : m_Registry(std::move(registry)),
            m_Camera(camera),
            m_State(state),
            m_ScriptBindings(std::move(scriptBindings)) { }
          

        entt::registry m_Registry;
        entt::entity m_Camera;
        SceneState m_State;
		std::vector<Scripts::Binding> m_ScriptBindings;
    };
}
