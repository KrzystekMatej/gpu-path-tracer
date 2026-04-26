#pragma once
#include <Core/Graphics/Cuda/Memory/DeviceBuffer1D.hpp>
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
		
		std::expected<void, Core::Utils::Error> Build(const HostBvhNode& root, const std::vector<Triangle>& triangles);
		std::expected<void, Core::Utils::Error> Free();


		DeviceBvhView GetView() const
		{
			return
			{
				m_Nodes.GetView<DeviceBvhNode>(),
				m_Triangles.GetView<Triangle>()
			};
		}

		size_t GetNodeCount() const { return m_Nodes.GetSize() / sizeof(DeviceBvhNode); }
		size_t GetTriangleCount() const { return m_Triangles.GetSize() / sizeof(Triangle); }
	private:
		Memory::DeviceBuffer1D m_Nodes;
		Memory::DeviceBuffer1D m_Triangles;
	};
}