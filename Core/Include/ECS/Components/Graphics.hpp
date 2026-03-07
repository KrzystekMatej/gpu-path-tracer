#pragma once
#include "Assets/Types.hpp"

namespace Core::ECS::Components
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
}