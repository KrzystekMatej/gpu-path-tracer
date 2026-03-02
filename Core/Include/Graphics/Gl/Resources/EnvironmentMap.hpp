#pragma once
#include "Graphics/Gl/Resources/Texture.hpp"

namespace Core::Graphics::Gl
{
	struct EnvironmentMap
	{
	public:
		EnvironmentMap(const EnvironmentMap&) = delete;
		EnvironmentMap& operator=(const EnvironmentMap&) = delete;
		EnvironmentMap(EnvironmentMap&&) noexcept = default;
		EnvironmentMap& operator=(EnvironmentMap&&) noexcept = default;

		EnvironmentMap(Texture background, Texture irradianceMap, Texture prefilteredMap)
			: m_Background(std::move(background)), m_IrradianceMap(std::move(irradianceMap)), m_PrefilteredMap(std::move(prefilteredMap)) {}

		const Texture& GetBackground() const { return m_Background; }
		const Texture& GetIrradianceMap() const { return m_IrradianceMap; }
		const Texture& GetPrefilteredMap() const { return m_PrefilteredMap; }
	private:
		Texture m_Background;
		Texture m_IrradianceMap;
		Texture m_PrefilteredMap;
	};
}