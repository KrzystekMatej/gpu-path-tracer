#include <Core/Graphics/Cuda/PathTracing/Memory/PathPool.hpp>

namespace Core::Graphics::Cuda
{
	std::expected<void, Core::Utils::Error> PathPool::Allocate(uint32_t pathCount, const Runtime::Stream& stream)
	{
		CORE_TRY_DISCARD(m_Samples.Allocate(pathCount, sizeof(Pixel), stream));
		CORE_TRY_DISCARD(m_RandomStates.Allocate(pathCount, sizeof(Random), stream));
		return {};
	}

	std::expected<void, Core::Utils::Error> PathPool::Free(const Runtime::Stream& stream)
	{
		auto sampleResult = m_Samples.Free(stream);
		auto randomStatesResult = m_RandomStates.Free(stream);
		
		CORE_TRY_DISCARD(sampleResult);
		CORE_TRY_DISCARD(randomStatesResult);
		return {};
	}
}
