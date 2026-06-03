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

	std::expected<void, Core::Utils::Error> DeviceBvh::Build(const HostBvhNode& root, uint32_t depth, uint32_t nodeCount, const std::vector<Triangle>& triangles)
	{
		std::vector<DeviceBvhNode> nodes;
		nodes.reserve(nodeCount);
		m_Depth = depth;
		FlattenBvh(root, nodes);
		CORE_TRY_DISCARD(m_Nodes.Allocate(nodes.size(), sizeof(DeviceBvhNode)));
		CORE_TRY_DISCARD(m_Nodes.UploadSync(nodes.data(), nodes.size()));
		CORE_TRY_DISCARD(m_Triangles.Allocate(triangles.size(), sizeof(Triangle)));
		CORE_TRY_DISCARD(m_Triangles.UploadSync(triangles.data(), triangles.size()));
		return {};
	}

	std::expected<void, Core::Utils::Error> DeviceBvh::Free()
	{
		auto nodeResult = m_Nodes.Free();
		auto triangleResult = m_Triangles.Free();
		if (!nodeResult)
			return std::unexpected(std::move(nodeResult).error());
		if (!triangleResult)
			return std::unexpected(std::move(triangleResult).error());
		return {};
	}
}