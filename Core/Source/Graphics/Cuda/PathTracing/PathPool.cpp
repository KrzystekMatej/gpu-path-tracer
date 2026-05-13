#include <Core/Graphics/Cuda/PathTracing/PathPool.hpp>

namespace Core::Graphics::Cuda
{
	std::expected<void, Utils::Error> PathPool::Allocate(size_t pathCount)
	{
    	CORE_TRY_VOID(Free());
		CORE_TRY_VOID(m_Samples.Allocate(pathCount, sizeof(Pixel)));
		CORE_TRY_VOID(m_Rays.Allocate(pathCount, sizeof(Ray)));
		CORE_TRY_VOID(m_Contributions.Allocate(pathCount, sizeof(Contribution)));
		CORE_TRY_VOID(m_RandomStates.Allocate(pathCount, sizeof(Random)));
		CORE_TRY_VOID(m_PathFlags.Allocate(pathCount, sizeof(PathFlags)));
		return {};
	}

	std::expected<void, Utils::Error> PathPool::Free()
	{
		CORE_TRY_VOID(m_Samples.Free());
		CORE_TRY_VOID(m_Rays.Free());
		CORE_TRY_VOID(m_Contributions.Free());
		CORE_TRY_VOID(m_RandomStates.Free());
		CORE_TRY_VOID(m_PathFlags.Free());
		return {};
	}
}