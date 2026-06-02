#pragma once
#include <Core/Utils/Error.hpp>
#include <Core/Graphics/Cuda/Memory/DeviceBuffer1D.hpp>
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
		
		std::expected<void, Core::Utils::Error> Allocate(size_t pathCount);
		std::expected<void, Core::Utils::Error> Free();


		PathPoolView GetView() const
		{
			return
			{
				m_Samples.GetView<Pixel>(),
				m_Rays.GetView<Ray>(),
				m_Contributions.GetView<Contribution>(),
				m_RandomStates.GetView<Random>(),
				m_PathFlags.GetView<PathFlags>()
			};
		}

		uint32_t GetPathCount() const { return m_Samples.GetSize(); }
	private:
		Memory::DeviceBuffer1D m_Samples;
		Memory::DeviceBuffer1D m_Rays;
		Memory::DeviceBuffer1D m_Contributions;
		Memory::DeviceBuffer1D m_RandomStates;
		Memory::DeviceBuffer1D m_PathFlags;
	};
}