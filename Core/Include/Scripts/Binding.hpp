#pragma once
#include "Utils/Guid.hpp"
#include "Scripts/Phase.hpp"

namespace Core::Scripts
{
    struct Binding
    {
		Binding() = default;
        Binding(Utils::Guid id, Phase phase)
			: id(id), phase(phase) {}

        Utils::Guid id;
        Phase phase;
    };
}