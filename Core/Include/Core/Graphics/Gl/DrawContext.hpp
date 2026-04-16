#pragma once
#include <Core/External/Glm.hpp>
#include <Core/Graphics/Gl/Resources/Mesh.hpp>
#include <Core/Graphics/Gl/Material.hpp>
#include <Core/Assets/Storage.hpp>
#include <Core/Graphics/Common/Light.hpp>
#include <Core/Graphics/Gl/Resources/EnvironmentMap.hpp>

namespace Core::Graphics::Gl
{
	struct DrawContext
	{
        DrawContext(
            const Assets::Storage& storage,
            const glm::mat4& model,
            const glm::mat4& view,
			const glm::mat4& projection,
            const glm::vec3 cameraPosition,
            const Resources::Mesh& mesh,
			const Material& material,
			const std::vector<Common::Light>& lights,
            const Resources::EnvironmentMap* environmentMap)
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
		const glm::vec3 cameraPosition;

        const Resources::Mesh& mesh;
        const Material& material;

		const std::vector<Common::Light>& lights;
		const Resources::EnvironmentMap* environmentMap = nullptr;
	};
}