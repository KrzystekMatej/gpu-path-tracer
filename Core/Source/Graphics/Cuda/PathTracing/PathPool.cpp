#include <Core/Graphics/Cuda/PathTracing/PathPool.hpp>

namespace Core::Graphics::Cuda
{
	std::expected<void, Utils::Error> PathPool::Allocate(size_t pathCount)
	{
        auto freeResult = Free();
        if (!freeResult)
            return std::unexpected(freeResult.error());

		if (auto result = m_Samples.Allocate(pathCount, sizeof(Sample)); !result)
			return std::unexpected(std::move(result).error());
		if (auto result = m_Rays.Allocate(pathCount, sizeof(Ray)); !result)
			return std::unexpected(std::move(result).error());
		if (auto result = m_Contributions.Allocate(pathCount, sizeof(Contribution)); !result)
			return std::unexpected(std::move(result).error());
		if (auto result = m_RandomStates.Allocate(pathCount, sizeof(Random)); !result)
			return std::unexpected(std::move(result).error());
		if (auto result = m_PathFlags.Allocate(pathCount, sizeof(PathFlags)); !result)
			return std::unexpected(std::move(result).error());
		return {};
	}

	std::expected<void, Utils::Error> PathPool::Free()
	{
		auto sampleResult = m_Samples.Free();
		auto rayResult = m_Rays.Free();
		auto contributionResult = m_Contributions.Free();
		auto randomResult = m_RandomStates.Free();
		auto pathFlagsResult = m_PathFlags.Free();

		if (!sampleResult)
			return std::unexpected(std::move(sampleResult).error());
		if (!rayResult)
			return std::unexpected(std::move(rayResult).error());
		if (!contributionResult)
			return std::unexpected(std::move(contributionResult).error());
		if (!randomResult)
			return std::unexpected(std::move(randomResult).error());
		if (!pathFlagsResult)
			return std::unexpected(std::move(pathFlagsResult).error());
		return {};
	}
}