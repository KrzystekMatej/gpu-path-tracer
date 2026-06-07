#pragma once
#include <Core/Utils/Error.hpp>
#include <Core/Graphics/Cuda/Runtime/DeviceBuffer1D.hpp>
#include <Core/Graphics/Cuda/PathTracing/PathPoolView.hpp>

namespace Core::Graphics::Cuda
{
	class PathPool
	{
	public:
		PathPool() = default;
		~PathPool() = default;
		PathPool(PathPool&& other) noexcept = default;
		PathPool& operator=(PathPool&& other) noexcept = default;
		PathPool(const PathPool&) = delete;
		PathPool& operator=(const PathPool&) = delete;
		
		std::expected<void, Core::Utils::Error> Allocate(uint32_t pathCount, const Runtime::Stream& stream = Runtime::Stream::Default());
		std::expected<void, Core::Utils::Error> Free(const Runtime::Stream& stream = Runtime::Stream::Default());


		PathPoolView GetView() const
		{
			return
			{
				m_Samples.GetView<Pixel>(),
				m_Contributions.GetView<Contribution>(),
				m_RandomStates.GetView<Random>(),
			};
		}

		uint32_t GetPathCount() const { return m_Samples.GetSize(); }
	private:
		Runtime::DeviceBuffer1D m_Samples;
		Runtime::DeviceBuffer1D m_Contributions;
		Runtime::DeviceBuffer1D m_RandomStates;
	};
}
