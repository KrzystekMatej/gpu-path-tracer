#pragma once
#include <vector>
#include <yaml-cpp/yaml.h>
#include "Scripts/Binding.hpp"

namespace Core::IO
{
	struct Scene
	{
		YAML::Node sceneRoot;
		std::vector<Scripts::Binding> scripts;
	};
}