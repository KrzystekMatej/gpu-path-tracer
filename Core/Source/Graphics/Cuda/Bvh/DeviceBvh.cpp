#include <Core/Graphics/Cuda/Bvh/DeviceBvh.hpp>

namespace Core::Graphics::Cuda
{
	namespace
	{
		uint32_t FlattenBvh(const HostBvhNode& node, std::vector<DeviceBvhNode>& nodes)
		{
			uint32_t nodeIndex = static_cast<uint32_t>(nodes.size());
			nodes.emplace_back(node.bounds, InvalidNodeIndex, InvalidNodeIndex, node.first, node.count);

			if (node.left && node.right)
			{
				nodes[nodeIndex].left = FlattenBvh(*node.left, nodes);
				nodes[nodeIndex].right = FlattenBvh(*node.right, nodes);
			}

			return nodeIndex;
		}
	}

	std::expected<void, Utils::Error> DeviceBvh::Build(const HostBvhNode& root, const std::vector<Triangle>& triangles)
	{
		std::vector<DeviceBvhNode> nodes;
		FlattenBvh(root, nodes);
		auto result = m_Nodes.Allocate(nodes.size(), sizeof(DeviceBvhNode));
		if (!result)
			return std::unexpected(std::move(result).error());
		result = m_Nodes.UploadSync(nodes.data(), nodes.size());
		if (!result)
			return std::unexpected(std::move(result).error());
		result = m_Triangles.Allocate(triangles.size(), sizeof(Triangle));
		if (!result)
			return std::unexpected(std::move(result).error());
		return m_Triangles.UploadSync(triangles.data(), triangles.size());
	}

	std::expected<void, Utils::Error> DeviceBvh::Free()
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