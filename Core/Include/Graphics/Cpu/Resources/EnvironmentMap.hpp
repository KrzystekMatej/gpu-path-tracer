#pragma once
#include "Graphics/Cpu/Resources/Texture.hpp"

namespace Core::Graphics::Cpu
{
	class EnvironmentMap
	{
	public:
		EnvironmentMap(const EnvironmentMap&) = delete;
		EnvironmentMap& operator=(const EnvironmentMap&) = delete;
		EnvironmentMap(EnvironmentMap&&) noexcept = default;
		EnvironmentMap& operator=(EnvironmentMap&&) noexcept = default;

		static EnvironmentMap Create(Texture background);

		const Texture& GetBackgroundTexture() const { return m_Background; }
	private:
		EnvironmentMap(Texture background, std::vector<float> conditionalCdf, std::vector<float> marginalCdf)
			: m_Background(std::move(background)), m_ConditionalCdf(std::move(conditionalCdf)), m_MarginalCdf(std::move(marginalCdf)) {}

		Texture m_Background;
		std::vector<float> m_ConditionalCdf;
		std::vector<float> m_MarginalCdf;
	};
}
