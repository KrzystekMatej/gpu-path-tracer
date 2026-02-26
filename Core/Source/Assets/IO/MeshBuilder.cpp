#include "Assets/IO/MeshBuilder.hpp"
#include <mikktspace.h>

namespace Core::Assets::IO
{
	namespace 
	{
		struct MikkContext
		{
			Core::Graphics::Cpu::Mesh* mesh;
		};

		static int GetNumFaces(const SMikkTSpaceContext* context)
		{
			auto* user = static_cast<MikkContext*>(context->m_pUserData);
			return static_cast<int>(user->mesh->indices.size() / 3);
		}

		static int GetNumVerticesOfFace(const SMikkTSpaceContext*, int)
		{
			return 3;
		}

		static void GetPosition(const SMikkTSpaceContext* context, float position[3], int face, int faceVertex)
		{
			auto* user = static_cast<MikkContext*>(context->m_pUserData);
			uint32_t index = user->mesh->indices[face * 3 + faceVertex];
			const auto& vertex = user->mesh->vertices[index];

			position[0] = vertex.position.x;
			position[1] = vertex.position.y;
			position[2] = vertex.position.z;
		}

		static void GetNormal(const SMikkTSpaceContext* context, float normal[3], int face, int faceVertex)
		{
			auto* user = static_cast<MikkContext*>(context->m_pUserData);
			uint32_t index = user->mesh->indices[face * 3 + faceVertex];
			const auto& vertex = user->mesh->vertices[index];

			normal[0] = vertex.normal.x;
			normal[1] = vertex.normal.y;
			normal[2] = vertex.normal.z;
		}

		static void GetTexCoord(const SMikkTSpaceContext* context, float uv[2], int face, int faceVertex)
		{
			auto* user = static_cast<MikkContext*>(context->m_pUserData);
			uint32_t index = user->mesh->indices[face * 3 + faceVertex];
			const auto& vertex = user->mesh->vertices[index];

			uv[0] = vertex.uv.x;
			uv[1] = vertex.uv.y;
		}

		static void SetTSpaceBasic(const SMikkTSpaceContext* context, const float tangent[3], float sign, int face, int faceVertex)
		{
			auto* user = static_cast<MikkContext*>(context->m_pUserData);
			uint32_t index = user->mesh->indices[face * 3 + faceVertex];
			auto& vertex = user->mesh->vertices[index];

			vertex.tangent = glm::vec4(tangent[0], tangent[1], tangent[2], sign);
		}

		void ComputeTangents(Core::Graphics::Cpu::Mesh& mesh)
		{
			MikkContext userData{ &mesh };

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

	Graphics::Cpu::Mesh BuildMesh(ParsedMesh parsedMesh)
	{
		Graphics::Cpu::Mesh mesh;
		mesh.vertices = std::move(parsedMesh.vertices);
		mesh.indices = std::move(parsedMesh.indices);
		ComputeTangents(mesh);
		return mesh;
	}
}