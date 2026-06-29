#include <Core/Graphics/Cuda/PathTracing/Memory/LightSampler.hpp>

namespace Core::Graphics::Cuda
{
    std::expected<void, Core::Utils::Error> LightSampler::BuildSync(const std::vector<Triangle>& triangles, const std::vector<Material>& materials, const Runtime::Stream& stream)
    {
        std::vector<LightTriangle> lights;
        std::vector<float> lightWeights;
        std::vector<uint32_t> lightIndices;

        for (size_t i = 0; i < triangles.size(); i++)
        {
            if (triangles[i].intersection.shadingModel != GlobalShadingModel::Emissive /*&& triangles[i].intersection.shadingModel != GlobalShadingModel::Normal*/)
                continue;
            
            // TODO: Instead of using the area as weight of the emissive triangle, we could use the total emitted power of the triangle
            // power = (pi * area * integral over the emission texture)
            float area = 0.5f * length(cross(make_float3(triangles[i].intersection.edge1), make_float3(triangles[i].intersection.edge2)));
            lightWeights.push_back(area);
            lightIndices.push_back(static_cast<uint32_t>(lights.size()));
            lights.push_back(LightTriangle{ 
                .edge1 = triangles[i].intersection.edge1,
                .edge2 = triangles[i].intersection.edge2,
                .v0 = triangles[i].intersection.v0,
                .index = static_cast<uint32_t>(i),
                .geometricNormal = triangles[i].shading.geometricNormal,
                .area = area,
                .uv0 = triangles[i].shading.uvs[0],
                .uvEdge1 = triangles[i].shading.uvs[1] - triangles[i].shading.uvs[0],
                .uvEdge2 = triangles[i].shading.uvs[2] - triangles[i].shading.uvs[0],
                .emission = materials[triangles[i].shading.material].emission
            });
        }
        
        CORE_TRY_DISCARD_CONTEXT(m_LightBuffer.Allocate(static_cast<uint32_t>(lights.size()), static_cast<uint32_t>(sizeof(LightTriangle)), stream), "Failed to allocate light buffer");
        CORE_TRY_DISCARD_CONTEXT(m_LightBuffer.Upload(lights.data(), static_cast<uint32_t>(lights.size()), stream), "Failed to upload light buffer");
        CORE_TRY_DISCARD_CONTEXT(m_AliasTable.BuildSync(lightWeights, lightIndices, stream), "Failed to build light table");
        CORE_TRY_DISCARD_CONTEXT(stream.Synchronize(), "Failed to synchronize after building light sampler");
        return {};
    }
    
    std::expected<void, Core::Utils::Error> LightSampler::Free(const Runtime::Stream& stream)
    {
        auto bufferResult = m_LightBuffer.Free(stream);
        auto aliasResult = m_AliasTable.Free(stream);

        CORE_TRY_DISCARD_CONTEXT(bufferResult, "Failed to free light buffer");
        CORE_TRY_DISCARD_CONTEXT(aliasResult, "Failed to free light alias table");

        return {};
    }
}