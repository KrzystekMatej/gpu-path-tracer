#pragma once
#include "External/Glm.hpp"
#include "Graphics/Gl/Resources/Mesh.hpp"
#include "Graphics/Gl/Material.hpp"
#include "Assets/Storage.hpp"
#include "Graphics/Light.hpp"
#include "Graphics/Gl/Resources/EnvironmentMap.hpp"


namespace Core::Graphics::Gl
{
	struct DrawContext
	{
        DrawContext(
            const Assets::Storage& storage,
            const glm::mat4& model,
            const glm::mat4& view,
			const glm::mat4& projection,
            const glm::vec3& cameraPosition,
            const Mesh& mesh,
			const Material& material,
			const std::vector<Light>& lights,
            const EnvironmentMap* environmentMap)
            : storage(storage),
              model(model),
              view(view),
			  projection(projection),
              cameraPosition(cameraPosition),
			  mesh(mesh),
			  material(material),
              lights(lights),
			environmentMap(environmentMap) {}

        const Assets::Storage& storage;

        const glm::mat4& model;
        const glm::mat4& view;
        const glm::mat4& projection;
		const glm::vec3& cameraPosition;

        const Mesh& mesh;
        const Material& material;

		const std::vector<Light>& lights;
		const EnvironmentMap* environmentMap = nullptr;
	};
}