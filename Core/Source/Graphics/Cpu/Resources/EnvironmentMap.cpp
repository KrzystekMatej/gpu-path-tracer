#include <Core/Graphics/Cpu/Resources/EnvironmentMap.hpp>

namespace Core::Graphics::Cpu
{
	EnvironmentMap EnvironmentMap::Create(Texture background)
	{
		return EnvironmentMap(std::move(background), {}, {});
	}
}