#pragma once
#include <Core/Graphics/Cuda/Runtime/Memory/DeviceBuffer1D.hpp>
#include <Core/Graphics/Cuda/Bvh/DeviceBvhView.hpp>
#include <Core/Graphics/Cuda/Bvh/HostBvh.hpp>

namespace Core::Graphics::Cuda
{
	class DeviceBvh
	{
	public:
		DeviceBvh() = default;
		~DeviceBvh() = default;
		DeviceBvh(DeviceBvh&& other) noexcept = default;
		DeviceBvh& operator=(DeviceBvh&& other) noexcept = default;
		DeviceBvh(const DeviceBvh&) = delete;
		DeviceBvh& operator=(const DeviceBvh&) = delete;
		
		std::expected<void, Core::Utils::Error> BuildSync(
			const HostBvhNode& root, 
			uint32_t depth, 
			uint32_t nodeCount, 
			const std::vector<Triangle>& triangles,
			const Runtime::Stream& stream = Runtime::Stream::Default());
		std::expected<void, Core::Utils::Error> Free(const Runtime::Stream& stream = Runtime::Stream::Default());


		IntersectionBvhView GetIntersectionView() const
		{
			return IntersectionBvhView(
				m_Nodes.GetView<DeviceBvhNode>(),
				m_IntersectionData.GetView<TriangleIntersection>()
			);
		}
		
		Runtime::DeviceBuffer1DView<TriangleShading> GetShadingView() const
		{
			return m_ShadingData.GetView<TriangleShading>();
		}

		size_t GetNodeCount() const { return m_Nodes.GetSize(); }
		size_t GetTriangleCount() const { return m_IntersectionData.GetSize(); }
		uint32_t GetDepth() const { return m_Depth; }
	private:
		Runtime::DeviceBuffer1D m_Nodes;
		Runtime::DeviceBuffer1D m_IntersectionData;
		Runtime::DeviceBuffer1D m_ShadingData;
		uint32_t m_Depth;
	};
}