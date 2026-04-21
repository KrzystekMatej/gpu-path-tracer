#include <Core/Utils/Math/TangentSpace.hpp>
#include <mikktspace.h>

namespace Core::Utils::Math
{
	namespace 
	{
		struct MikkContext
		{
			std::vector<Graphics::Vertex>& vertices;
			const std::vector<uint32_t>& indices;
		};

		int GetNumFaces(const SMikkTSpaceContext* context)
		{
			auto* user = static_cast<MikkContext*>(context->m_pUserData);
			return static_cast<int>(user->indices.size() / 3);
		}

		int GetNumVerticesOfFace(const SMikkTSpaceContext*, int)
		{
			return 3;
		}

		void GetPosition(const SMikkTSpaceContext* context, float position[3], int face, int faceVertex)
		{
			auto* user = static_cast<MikkContext*>(context->m_pUserData);
			uint32_t index = user->indices[face * 3 + faceVertex];
			const auto& vertex = user->vertices[index];

			position[0] = vertex.position.x;
			position[1] = vertex.position.y;
			position[2] = vertex.position.z;
		}

		void GetNormal(const SMikkTSpaceContext* context, float normal[3], int face, int faceVertex)
		{
			auto* user = static_cast<MikkContext*>(context->m_pUserData);
			uint32_t index = user->indices[face * 3 + faceVertex];
			const auto& vertex = user->vertices[index];

			normal[0] = vertex.normal.x;
			normal[1] = vertex.normal.y;
			normal[2] = vertex.normal.z;
		}

		void GetTexCoord(const SMikkTSpaceContext* context, float uv[2], int face, int faceVertex)
		{
			auto* user = static_cast<MikkContext*>(context->m_pUserData);
			uint32_t index = user->indices[face * 3 + faceVertex];
			const auto& vertex = user->vertices[index];

			uv[0] = vertex.uv.x;
			uv[1] = vertex.uv.y;
		}

		void SetTSpaceBasic(const SMikkTSpaceContext* context, const float tangent[3], float sign, int face, int faceVertex)
		{
			auto* user = static_cast<MikkContext*>(context->m_pUserData);
			uint32_t index = user->indices[face * 3 + faceVertex];
			auto& vertex = user->vertices[index];

			vertex.tangent = glm::vec4(tangent[0], tangent[1], tangent[2], sign);
		}

		void ComputeTangents(MikkContext& userData)
		{
			SMikkTSpaceInterface interface{};
			interface.m_getNumFaces = GetNumFaces;
			interface.m_getNumVerticesOfFace = GetNumVerticesOfFace;
			interface.m_getPosition = GetPosition;
			interface.m_getNormal = GetNormal;
			interface.m_getTexCoord = GetTexCoord;
			interface.m_setTSpaceBasic = SetTSpaceBasic;
			interface.m_setTSpace = nullptr;

			SMikkTSpaceContext context{};
			context.m_pInterface = &interface;
			context.m_pUserData = &userData;

			genTangSpaceDefault(&context);
		}
	}

	void GenerateTangentSpace(std::vector<Graphics::Vertex>& vertices, const std::vector<uint32_t>& indices)
	{
		MikkContext userData { vertices, indices };
		ComputeTangents(userData);
	}
}