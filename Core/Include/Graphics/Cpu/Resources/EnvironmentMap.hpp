#pragma once
#include "Graphics/Cpu/Resources/Texture.hpp"

namespace Core::Graphics::Cpu
{
	class EnvironmentMap
	{
	public:
		static EnvironmentMap Create(Texture background);

		const Texture& GetBackgroundTexture() const { return m_Background; }
	private:
		EnvironmentMap(const EnvironmentMap&) = delete;
		EnvironmentMap& operator=(const EnvironmentMap&) = delete;
		EnvironmentMap(EnvironmentMap&&) noexcept = default;
		EnvironmentMap& operator=(EnvironmentMap&&) noexcept = default;

		EnvironmentMap(Texture background, std::vector<float> conditionalCdf, std::vector<float> marginalCdf)
			: m_Background(std::move(background)), m_ConditionalCdf(std::move(conditionalCdf)), m_MarginalCdf(std::move(marginalCdf)) {}

		Texture m_Background;
		std::vector<float> m_ConditionalCdf;
		std::vector<float> m_MarginalCdf;
	};
}
