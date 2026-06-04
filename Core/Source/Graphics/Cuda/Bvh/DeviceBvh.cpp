#include <Core/Graphics/Cuda/Bvh/DeviceBvh.hpp>

namespace Core::Graphics::Cuda
{
	namespace
	{
		uint32_t FlattenBvh(const HostBvhNode& node, std::vector<DeviceBvhNode>& nodes)
		{
			uint32_t nodeIndex = static_cast<uint32_t>(nodes.size());
			nodes.emplace_back(node.bounds, DeviceBvhNode::InvalidIndex, DeviceBvhNode::InvalidIndex, node.first, node.count);

			if (node.left && node.right)
			{
				nodes[nodeIndex].left = FlattenBvh(*node.left, nodes);
				nodes[nodeIndex].right = FlattenBvh(*node.right, nodes);
			}

			return nodeIndex;
		}
	}

	std::expected<void, Core::Utils::Error> DeviceBvh::Build(
		const HostBvhNode& root, 
		uint32_t depth, 
		uint32_t nodeCount, 
		const std::vector<Triangle>& triangles, 
		const Runtime::Stream& stream)
	{
		std::vector<DeviceBvhNode> nodes;
		nodes.reserve(nodeCount);
		m_Depth = depth;
		FlattenBvh(root, nodes);
		CORE_TRY_DISCARD(m_Nodes.Allocate(static_cast<uint32_t>(nodes.size()), sizeof(DeviceBvhNode), stream));
		CORE_TRY_DISCARD(m_Nodes.Upload(nodes.data(), static_cast<uint32_t>(nodes.size()), stream));
		CORE_TRY_DISCARD(m_Triangles.Allocate(static_cast<uint32_t>(triangles.size()), sizeof(Triangle), stream));
		CORE_TRY_DISCARD(m_Triangles.Upload(triangles.data(), static_cast<uint32_t>(triangles.size()), stream));
		CORE_TRY_DISCARD(stream.Synchronize());
		return {};
	}

	std::expected<void, Core::Utils::Error> DeviceBvh::Free(const Runtime::Stream& stream)
	{
		auto nodeResult = m_Nodes.Free(stream);
		auto triangleResult = m_Triangles.Free(stream);
		if (!nodeResult)
			return std::unexpected(std::move(nodeResult).error());
		if (!triangleResult)
			return std::unexpected(std::move(triangleResult).error());
		return {};
	}
}