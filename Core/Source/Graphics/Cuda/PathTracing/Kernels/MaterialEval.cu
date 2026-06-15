#include <Core/Graphics/Cuda/PathTracing/Kernels/LaunchUtils.cuh>
#include <Core/Graphics/Cuda/PathTracing/Kernels/Launchers.hpp>
#include <Core/Graphics/Cuda/Utils/Math.hpp>
#include <Core/Graphics/Cuda/PathTracing/PathTracerDefaults.hpp>

namespace Core::Graphics::Cuda::Kernels
{
	struct PartiallyResolvedHit
	{
		uint32_t material;
		float2 uv;
	};

	struct ResolvedHit
	{
		uint32_t material;
		float3 geometricNormal;
		float2 uv;
		float3 normal;
		float4 tangent;
		float area;
	};
	
	__device__ __forceinline__ float3 FixGeometricNormal(float3 geometricNormal, float3 rayDirection)
	{
		if (dot(geometricNormal, rayDirection) > 0.0f)
			return -geometricNormal;
		return geometricNormal;
	}

	__device__ __forceinline__ PartiallyResolvedHit LoadPartiallyResolvedHit(
		MaterialEvalQueueView materialEvalQueue, 
		uint32_t queueIndex, 
		Runtime::DeviceBuffer1DView<TriangleShading> triangles)
	{
		const HitData hitData = materialEvalQueue.GetHitData(queueIndex);
		const TriangleShading triangle = triangles.At(hitData.triangle);

		float b0 = 1.0f - hitData.u - hitData.v;
		float b1 = hitData.u;
		float b2 = hitData.v;

		PartiallyResolvedHit hit;
		hit.material = triangle.material;
		hit.uv =
			b0 * triangle.uvs[0] +
			b1 * triangle.uvs[1] +
			b2 * triangle.uvs[2];

		return hit;
	}
	
	__device__ __forceinline__ ResolvedHit LoadResolvedHit(
		float3 rayDirection,
		MaterialEvalQueueView materialEvalQueue, 
		uint32_t queueIndex, 
		Runtime::DeviceBuffer1DView<TriangleShading> triangles)
	{
		const HitData hitData = materialEvalQueue.GetHitData(queueIndex);
		const TriangleShading triangle = triangles.At(hitData.triangle);

		float b0 = 1.0f - hitData.u - hitData.v;
		float b1 = hitData.u;
		float b2 = hitData.v;

		ResolvedHit hit;
		hit.material = triangle.material;
		hit.geometricNormal = FixGeometricNormal(triangle.geometricNormal, rayDirection);
		hit.uv =
			b0 * triangle.uvs[0] +
			b1 * triangle.uvs[1] +
			b2 * triangle.uvs[2];

		hit.normal =
			b0 * make_float3(triangle.normals[0]) +
			b1 * make_float3(triangle.normals[1]) +
			b2 * make_float3(triangle.normals[2]);

		hit.tangent =
			b0 * triangle.tangents[0] +
			b1 * triangle.tangents[1] +
			b2 * triangle.tangents[2];

		return hit;
	}

	__device__ __forceinline__ float3 LoadTangentSpaceNormal(TextureView<float4> normalMap, float2 uv)
	{
		float3 normalMapSample = make_float3(normalMap.Sample(uv));
		return normalize(normalMapSample * 2.0f - 1.0f);
	}
	

	__device__ __forceinline__ Math::Frame GetShadingFrame(const ResolvedHit& hit, TextureView<float4> normalMap)
	{
		float3 baseTangent = make_float3(
			hit.tangent.x,
			hit.tangent.y,
			hit.tangent.z
		);

		float tangentSign = hit.tangent.w < 0.0f ? -1.0f : 1.0f;
		Math::Frame baseFrame = Math::BuildFrame(hit.normal, baseTangent, tangentSign);

		float3 tangentSpaceNormal = LoadTangentSpaceNormal(normalMap, hit.uv);

		float3 shadingNormal = 
			tangentSpaceNormal.x * baseFrame.tangent +
			tangentSpaceNormal.y * baseFrame.bitangent +
			tangentSpaceNormal.z * baseFrame.normal;

		return Math::BuildFrame(shadingNormal, baseTangent, tangentSign);
	}

	__device__ __forceinline__ float Luminance(float3 color)
	{
		return 0.2126f * color.x + 0.7152f * color.y + 0.0722f * color.z;
	}
	
	struct PathContinuation
	{
		bool terminated;
		uint32_t depth;
		float3 throughput;
	};

	__device__ __forceinline__ PathContinuation ComputePathContinuation(const Path& path, uint32_t pathDepthLimit, float3 scatterWeight, float rrSample)
	{
		uint32_t nextDepth = path.depth + 1;
		if (nextDepth >= pathDepthLimit)
			return { true, nextDepth, make_float3(0.0f) };

		float3 nextThroughput = path.throughput * scatterWeight;
		
		if (nextDepth < PathTracerDefaults::RussianRouletteStartDepth)
			return { false, nextDepth, nextThroughput };

		float survivalProbability = clamp(Math::MaxComponent(nextThroughput), 0.0f, 1.0f);

		if (rrSample > survivalProbability)
			return { true, nextDepth, make_float3(0.0f) };

		return { false, nextDepth, nextThroughput / survivalProbability };
	}

	__device__ __forceinline__ Ray SpawnScatteredRay(const Ray& ray, float3 nextDirection, float3 geometricNormal)
	{
		Ray spawnedRay;
		spawnedRay.origin = ray.origin + ray.direction * ray.tMax + geometricNormal * 1e-4f;
		spawnedRay.direction = nextDirection;
		spawnedRay.tMin = PathTracerDefaults::MinT;
		spawnedRay.tMax = PathTracerDefaults::MaxT;
		return spawnedRay;
	}
	
	__device__ __forceinline__ void AddRadiance(PathPoolView pathPool, PathContribution path, float3 emission, Runtime::DeviceBuffer1DView<float4> accumulationBuffer)
	{
		const Pixel pixel = pathPool.pixels.At(path.index);
		const float3 radiance = path.throughput * emission;
		atomicAdd(&accumulationBuffer.At(pixel.index).x, radiance.x);
		atomicAdd(&accumulationBuffer.At(pixel.index).y, radiance.y);
		atomicAdd(&accumulationBuffer.At(pixel.index).z, radiance.z);
	}
	
	template<bool UseNextEventEstimation>
	__global__ void EvaluateNormalKernel(
		PathPoolView pathPool, 
		MaterialEvalQueueView materialEvalQueue, 
		Runtime::DeviceBuffer1DView<TriangleShading> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		RegenQueueView regenQueue,
		Runtime::DeviceBuffer1DView<float4> accumulationBuffer)
	{
		const uint32_t queueIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (queueIndex >= materialEvalQueue.GetSize()) return;
		
		const PathContribution path = materialEvalQueue.GetPathContribution(queueIndex);
		const float3 rayDirection = materialEvalQueue.GetRayDirection(queueIndex);
		const ResolvedHit hit = LoadResolvedHit(rayDirection, materialEvalQueue, queueIndex, triangles);
		
		// geometric normal can be used to see the orientation of triangle surfaces
		// float3 normal = hit.geometricNormal;

		// shading normal can be used to see the effect of normal mapping
		// const Material material = materials.At(hit.triangle.material);
		// float3 normal = GetShadingFrame(hit, material.normal).normal;

		// visual normal
		float3 normal = hit.normal;
		AddRadiance(pathPool, path, normal * 0.5f + 0.5f, accumulationBuffer);
		regenQueue.Push(path.index);
	}
	
	__device__ __forceinline__ float3 SampleCosineWeightedHemisphere(float2 u)
	{
		float phi = 2.0f * CUDART_PI_F * u.x;
		float cosTheta = sqrtf(u.y);
		float sinTheta = sqrtf(1.0f - u.y);
		
		return make_float3(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta);
	}

	__device__ void EvaluateDiffuseIndirect(
		Path path, 
		const Ray& ray,
		uint32_t pathDepthLimit,
		float3 geometricNormal,
		const Math::Frame& shadingFrame, 
		float3 randomSamples,
		float3 albedo,
		RegenQueueView regenQueue,
		RayQueueView nextRayQueue)
	{
        const float2 directionSample = make_float2(randomSamples.x, randomSamples.y);
		const float rrSample = randomSamples.z; 
		
		const float3 omegaI = shadingFrame.FromLocal(SampleCosineWeightedHemisphere(directionSample));
		
		const PathContinuation continuation = ComputePathContinuation(path, pathDepthLimit, albedo, rrSample);

		if (continuation.terminated)
		{
			regenQueue.Push(path.index);
			return;
		}
		
		const Ray scatteredRay = SpawnScatteredRay(ray, omegaI, geometricNormal);
		path.depth = continuation.depth;
		path.throughput = continuation.throughput;
		path.lastScatterWasDelta = false;
		nextRayQueue.Push(path, scatteredRay);
	}

	template<bool UseNextEventEstimation>
	__global__ void EvaluateDiffuseKernel(
		PathPoolView pathPool, 
		MaterialEvalQueueView materialEvalQueue, 
		Runtime::DeviceBuffer1DView<TriangleShading> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		LightSamplerView lightSampler,
		RegenQueueView regenQueue,
		RayQueueView nextRayQueue)
	{
		const uint32_t queueIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (queueIndex >= materialEvalQueue.GetSize()) return;
		
		Path path = materialEvalQueue.GetPath(queueIndex);
		const Ray ray = materialEvalQueue.GetRay(queueIndex);
		const ResolvedHit hit = LoadResolvedHit(ray.direction, materialEvalQueue, queueIndex, triangles);

		const Material material = materials.At(hit.material);
		const float3 albedo = make_float3(material.color.Sample(hit.uv));
		
		const Math::Frame shadingFrame = GetShadingFrame(hit, material.normal);
        
        Random random = pathPool.randoms.At(path.index);
        const float3 samples = random.NextFloat3();
		pathPool.randoms.At(path.index) = random;
		EvaluateDiffuseIndirect(path, ray, pathDepthLimit, hit.geometricNormal, shadingFrame, samples, albedo, regenQueue, nextRayQueue);
	}
	
	__global__ void EvaluateMirrorKernel(
		PathPoolView pathPool, 
		MaterialEvalQueueView materialEvalQueue, 
		Runtime::DeviceBuffer1DView<TriangleShading> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		RegenQueueView regenQueue,
		RayQueueView nextRayQueue)
	{
		const uint32_t queueIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (queueIndex >= materialEvalQueue.GetSize()) return;
		
		Path path = materialEvalQueue.GetPath(queueIndex);
		const Ray ray = materialEvalQueue.GetRay(queueIndex);
		const ResolvedHit hit = LoadResolvedHit(ray.direction, materialEvalQueue, queueIndex, triangles);

		const Material material = materials.At(hit.material);
		const float3 reflectance = make_float3(material.specular.Sample(hit.uv));

		const Math::Frame shadingFrame = GetShadingFrame(hit, material.normal);
		
		Random random = pathPool.randoms.At(path.index);
		const float rrSample = random.NextFloat();
		pathPool.randoms.At(path.index) = random;

		const float3 omegaI = reflect(ray.direction, shadingFrame.normal);

		const PathContinuation continuation = ComputePathContinuation(path, pathDepthLimit, reflectance, rrSample);

		if (continuation.terminated)
		{
			regenQueue.Push(path.index);
			return;
		}
		
		const Ray scatteredRay = SpawnScatteredRay(ray, omegaI, hit.geometricNormal);
		path.depth = continuation.depth;
		path.throughput = continuation.throughput;
		path.lastScatterWasDelta = true;
		nextRayQueue.Push(path, scatteredRay);
	}
	
	__device__ __forceinline__ float3 SamplePhongLobe(float3 reflected, float exponent, float2 u)
	{
		float phi = 2.0f * CUDART_PI_F * u.x;
		float cosTheta = powf(u.y, 1.0f / (exponent + 1.0f));
		float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);

		float3 sampled = make_float3(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta);
		
		return Math::BuildFrame(reflected).FromLocal(sampled);
	}
	
	__device__ __forceinline__ float LoadShininess(TextureView<float> shininessMap, float2 uv)
	{
		return shininessMap.Sample(uv.x, uv.y) * (MaterialDefaults::MaxShininess - MaterialDefaults::MinShininess) + MaterialDefaults::MinShininess;
	}

	__device__ void EvaluatePhongIndirect(
		Path path,
		const Ray& ray,
		uint32_t pathDepthLimit,
		float3 geometricNormal,
		const Math::Frame& shadingFrame,
		float4 randomSamples,
		float3 albedo,
		float3 specular,
		float shininess,
		RegenQueueView regenQueue,
		RayQueueView nextRayQueue)
	{
		const float2 directionSample = make_float2(randomSamples.x, randomSamples.y);
		const float lobeSample = randomSamples.z;
		const float rrSample = randomSamples.w;
		
		float diffuseWeight = Luminance(albedo);
		float specularWeight = Luminance(specular);
		float weightSum = diffuseWeight + specularWeight;

		if (weightSum <= 0.0f)
		{
			regenQueue.Push(path.index);
			return;
		}

		float diffuseSelectionProbability = diffuseWeight / weightSum;
		float specularSelectionProbability = 1.0f - diffuseSelectionProbability;

		float3 reflectionDirection = normalize(reflect(ray.direction, shadingFrame.normal));

		float3 omegaI;

		if (lobeSample < diffuseSelectionProbability)
		{
			omegaI = shadingFrame.FromLocal(SampleCosineWeightedHemisphere(directionSample));
		}
		else
		{
			omegaI = SamplePhongLobe(reflectionDirection, shininess, directionSample);
		}

		float cosThetaI = fmaxf(dot(shadingFrame.normal, omegaI), 0.0f);
		float geometricCosThetaI = fmaxf(dot(geometricNormal, omegaI), 0.0f);

		if (cosThetaI <= 0.0f || geometricCosThetaI <= 0.0f)
		{
			regenQueue.Push(path.index);
			return;
		}

		float cosThetaR = fmaxf(dot(reflectionDirection, omegaI), 0.0f);
		float phongLobeOverTwoPi = Math::InvTwoPi * powf(cosThetaR, shininess); 
		// this simplication can't be used in case of energy-normalized modified phong 
		// (in such case the normalization term is sampled from precomputed textures since it's not trivial to compute on the fly)

		float diffusePdf = cosThetaI * Math::InvPi;
		float specularPdf = (shininess + 1.0f) * phongLobeOverTwoPi;

		float pdf = diffuseSelectionProbability * diffusePdf + specularSelectionProbability * specularPdf;

		if (pdf <= 0.0f)
		{
			regenQueue.Push(path.index);
			return;
		}

		float3 diffuseBrdf = albedo * Math::InvPi;
		float3 specularBrdf = specular * (shininess + 2.0f) * phongLobeOverTwoPi;

		const PathContinuation continuation = ComputePathContinuation(path, pathDepthLimit, (diffuseBrdf + specularBrdf) * (cosThetaI / pdf), rrSample);

		if (continuation.terminated)
		{
			regenQueue.Push(path.index);
			return;
		}
		
		const Ray scatteredRay = SpawnScatteredRay(ray, omegaI, geometricNormal);
		path.depth = continuation.depth;
		path.throughput = continuation.throughput;
		path.lastScatterWasDelta = false;
		nextRayQueue.Push(path, scatteredRay);
	}

	template<bool UseNextEventEstimation>
	__global__ void EvaluatePhongKernel(
		PathPoolView pathPool, 
		MaterialEvalQueueView materialEvalQueue, 
		Runtime::DeviceBuffer1DView<TriangleShading> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		LightSamplerView lightSampler,
		RegenQueueView regenQueue,
		RayQueueView nextRayQueue)
	{
		const uint32_t queueIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (queueIndex >= materialEvalQueue.GetSize()) return;
		
		Path path = materialEvalQueue.GetPath(queueIndex);
		const Ray ray = materialEvalQueue.GetRay(queueIndex);
		const ResolvedHit hit = LoadResolvedHit(ray.direction, materialEvalQueue, queueIndex, triangles);

		const Material material = materials.At(hit.material);
		const float3 albedo = make_float3(material.color.Sample(hit.uv));
		const float3 specular = make_float3(material.specular.Sample(hit.uv));
		const float shininess = LoadShininess(material.shininess, hit.uv);

		const Math::Frame shadingFrame = GetShadingFrame(hit, material.normal);

		Random random = pathPool.randoms.At(path.index);
        const float4 samples = random.NextFloat4();
		pathPool.randoms.At(path.index) = random;

		EvaluatePhongIndirect(path, ray, pathDepthLimit, hit.geometricNormal, shadingFrame, samples, albedo, specular, shininess, regenQueue, nextRayQueue);
	}
	
	__device__ __forceinline__ float2 SampleUniformDiskPolar(float2 u)
	{
		float r = sqrtf(u.x);
		float theta = 2.0f * CUDART_PI_F * u.y;
		return make_float2(r * cosf(theta), r * sinf(theta));
	}
	
	__device__ __forceinline__ float3 TrowbridgeSampleWm(float3 w, float2 alpha, float2 u)
	{
		float3 wh = normalize(make_float3(alpha.x * w.x, alpha.y * w.y, w.z));
		if (wh.z < 0)
			wh = -wh;
		
		float3 tangent = (wh.z < 0.999f) ? normalize(cross(wh, make_float3(0.0f, 0.0f, 1.0f))) : make_float3(1.0f, 0.0f, 0.0f);
		float3 bitangent = cross(wh, tangent);

		float2 diskSample = SampleUniformDiskPolar(u);
		float h = sqrtf(1.0f - diskSample.x * diskSample.x);
		diskSample.y = lerp(h, diskSample.y, (1 + wh.z) * 0.5f);
		
		float pz = sqrtf(fmaxf(0.0f, 1.0f - dot(diskSample, diskSample)));
		float3 nh = diskSample.x * tangent + diskSample.y * bitangent + pz * wh;
		return normalize(make_float3(alpha.x * nh.x, alpha.y * nh.y, fmaxf(1e-6f, nh.z)));
	}

	__device__ __forceinline__ float Lambda(float3 w, float alpha)
	{
		float tan2Theta = Math::Tan2Theta(w);
		if (tan2Theta == CUDART_INF_F) 
			return 0.0f;
		float alpha2 = Math::Sqr(Math::CosPhi(w) * alpha) + Math::Sqr(Math::SinPhi(w) * alpha);
		return (sqrtf(1.0f + alpha2 * tan2Theta) - 1.0f) * 0.5f;
	}
	
	__device__ __forceinline__ float G1(float3 w, float alpha)
	{
		return 1.0f / (1.0f + Lambda(w, alpha));
	}
	
	__device__ __forceinline__ float G(float3 wo, float3 wi, float alpha)
	{
		return 1 / (1 + Lambda(wo, alpha) + Lambda(wi, alpha));
	}
	
	__device__ __forceinline__ float D(float3 wm, float alpha)
	{
		float tan2Theta = Math::Tan2Theta(wm);
		if (tan2Theta == CUDART_INF_F)
			return 0.0f;
		float cos4Theta = Math::Sqr(Math::Cos2Theta(wm));
		if (cos4Theta < 1e-16f) return 0.0f;
		float e = tan2Theta * (Math::Sqr(Math::CosPhi(wm) / alpha) + Math::Sqr(Math::SinPhi(wm) / alpha));
		return 1 / (CUDART_PI_F * Math::Sqr(alpha) * cos4Theta * Math::Sqr(1 + e));
	}

	__device__ __forceinline__ float D(float3 w, float3 wm, float alpha)
	{
		return G1(w, alpha) / Math::AbsCosTheta(w) * D(wm, alpha) * fabsf(dot(w, wm));
	}

	__device__ __forceinline__ float3 FresnelSchlick(float3 f0, float cosThetaH)
	{
		return f0 + (1.0f - f0) * powf(1.0f - cosThetaH, 5.0f);
	}

	__device__ void EvaluateGgxIndirect(
		Path path, 
		const Ray& ray,
		uint32_t pathDepthLimit,
		float3 geometricNormal,
		const Math::Frame& shadingFrame, 
		float4 randomSamples,
		float3 baseColor, 
		float alpha, 
		float metallic,
		float ior,
		RegenQueueView regenQueue,
		RayQueueView nextRayQueue)
	{
		const float2 directionSample = make_float2(randomSamples.x, randomSamples.y);
		const float lobeSample = randomSamples.z;
		const float rrSample = randomSamples.w;
		
		alpha = fmaxf(alpha, 0.001f); // in the future we should treat alpha=0 as a special case - effectively smooth surface
		float3 wo = shadingFrame.ToLocal(make_float3(0.0f) - ray.direction);

		if (wo.z <= 0.0f)
		{
			regenQueue.Push(path.index);
			return;
		}

		float dielectricF0 = Math::Sqr((path.currentMediumIor - ior) / (path.currentMediumIor + ior));
		float3 f0 = lerp(make_float3(dielectricF0), baseColor, metallic);

		float3 diffuseColor = baseColor * (1.0f - metallic);

		float diffuseWeight = Luminance(diffuseColor);
		float specularWeight = Luminance(f0);

		float weightSum = diffuseWeight + specularWeight;
		if (weightSum <= 1e-6f)
		{
			regenQueue.Push(path.index);
			return;
		}

		float diffuseSelectionProbability = diffuseWeight / weightSum;
		float specularSelectionProbability = specularWeight / weightSum;
		
		float3 wi;

		if (lobeSample < diffuseSelectionProbability)
		{
			wi = SampleCosineWeightedHemisphere(directionSample);
		}
		else
		{
			float3 sampledWm = TrowbridgeSampleWm(wo, make_float2(alpha, alpha), directionSample);
			wi = reflect(-wo, sampledWm);
		}

		if (wi.z <= 0.0f)
		{
			regenQueue.Push(path.index);
			return;
		}
		
		float3 wm = normalize(wi + wo);
		
		if (wm.z <= 0.0f)
		{
			regenQueue.Push(path.index);
			return;
		}

		float woDotWm = dot(wo, wm);
		
		if (woDotWm <= 1e-6f)
		{
			regenQueue.Push(path.index);
			return;
		}

		float cosThetaI = Math::CosTheta(wi);
		float cosThetaO = Math::CosTheta(wo);

		float3 fresnel = FresnelSchlick(f0, clamp(woDotWm, 0.0f, 1.0f));
		
		float3 diffuseBsdf = (1.0f - fresnel) * diffuseColor * Math::InvPi;
		float3 ggxBsdf = fresnel * D(wm, alpha) * G(wo, wi, alpha) / (4.0f * cosThetaI * cosThetaO);
	
		float diffusePdf = cosThetaI * Math::InvPi;
		float ggxPdf = D(wo, wm, alpha) / (4.0f * woDotWm);
		float pdf = diffuseSelectionProbability * diffusePdf + specularSelectionProbability * ggxPdf;
		
		if (pdf <= 1e-8f)
		{
			regenQueue.Push(path.index);
			return;
		}
		
		float3 worldWi = shadingFrame.FromLocal(wi);
		if (dot(worldWi, geometricNormal) <= 0.0f)
		{
			regenQueue.Push(path.index);
			return;
		}
		
		const PathContinuation continuation = ComputePathContinuation(path, pathDepthLimit, (diffuseBsdf + ggxBsdf) * (cosThetaI / pdf), rrSample);
		
		if (continuation.terminated)
		{
			regenQueue.Push(path.index);
			return;
		}
		
		const Ray scatteredRay = SpawnScatteredRay(ray, worldWi, geometricNormal);
		path.depth = continuation.depth;
		path.throughput = continuation.throughput;
		path.lastScatterWasDelta = false;
		nextRayQueue.Push(path, scatteredRay);
	}

	template<bool UseNextEventEstimation>
	__global__ void EvaluateGgxKernel(
		PathPoolView pathPool, 
		MaterialEvalQueueView materialEvalQueue, 
		Runtime::DeviceBuffer1DView<TriangleShading> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		LightSamplerView lightSampler,
		RegenQueueView regenQueue,
		RayQueueView nextRayQueue)
	{
		const uint32_t queueIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (queueIndex >= materialEvalQueue.GetSize()) return;
		
		const Path path = materialEvalQueue.GetPath(queueIndex);
		const Ray ray = materialEvalQueue.GetRay(queueIndex);
		const ResolvedHit hit = LoadResolvedHit(ray.direction, materialEvalQueue, queueIndex, triangles);

		const Material material = materials.At(hit.material);
		const float3 baseColor = make_float3(material.color.Sample(hit.uv));
		const float3 rma = make_float3(material.rma.Sample(hit.uv));
		float alpha = rma.x * rma.x;
		float metallic = rma.y;

		const Math::Frame shadingFrame = GetShadingFrame(hit, material.normal);

		Random random = pathPool.randoms.At(path.index);
		const float4 samples = random.NextFloat4();
		pathPool.randoms.At(path.index) = random;

		EvaluateGgxIndirect(path, ray, pathDepthLimit, hit.geometricNormal, shadingFrame, samples, baseColor, alpha, metallic, material.ior, regenQueue, nextRayQueue);
	}

	template<bool UseNextEventEstimation>
	__global__ void EvaluateEmissiveKernel(
		PathPoolView pathPool, 
		MaterialEvalQueueView materialEvalQueue, 
		Runtime::DeviceBuffer1DView<TriangleShading> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		RegenQueueView regenQueue,
		Runtime::DeviceBuffer1DView<float4> accumulationBuffer)
	{
		const uint32_t queueIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (queueIndex >= materialEvalQueue.GetSize()) return;
		
		const PathContribution path = materialEvalQueue.GetPathContribution(queueIndex);
		const PartiallyResolvedHit hit = LoadPartiallyResolvedHit(materialEvalQueue, queueIndex, triangles);
		const Material material = materials.At(hit.material);
		
		const float3 emission = make_float3(material.emission.Sample(hit.uv));
		AddRadiance(pathPool, path, emission, accumulationBuffer);
		regenQueue.Push(path.index);
	}

	void EvaluateNormalMaterial(
		bool useNextEventEstimation,
		cudaStream_t stream,
		PathPoolView pathPool,
		MaterialEvalQueueView materialEvalQueue,
		Runtime::DeviceBuffer1DView<TriangleShading> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		LightSamplerView lightSampler,
		RegenQueueView regenQueue,
		Runtime::DeviceBuffer1DView<float4> accumulationBuffer,
		RayQueueView nextRayQueue)
	{
		dim3 block(LaunchUtils::MaterialEvalThreadsPerBlock);
		dim3 grid(LaunchUtils::GetBlockCount(materialEvalQueue.GetCapacity(), block.x));
		
		if (useNextEventEstimation)
			EvaluateNormalKernel<true><<<grid, block, 0, stream>>>(pathPool, materialEvalQueue, triangles, materials, regenQueue, accumulationBuffer);
		else
			EvaluateNormalKernel<false><<<grid, block, 0, stream>>>(pathPool, materialEvalQueue, triangles, materials, regenQueue, accumulationBuffer);
	}

	void EvaluateDiffuseMaterial(
		bool useNextEventEstimation,
		cudaStream_t stream,
		PathPoolView pathPool,
		MaterialEvalQueueView materialEvalQueue,
		Runtime::DeviceBuffer1DView<TriangleShading> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		LightSamplerView lightSampler,
		RegenQueueView regenQueue,
		Runtime::DeviceBuffer1DView<float4> accumulationBuffer,
		RayQueueView nextRayQueue)
	{
		dim3 block(LaunchUtils::MaterialEvalThreadsPerBlock);
		dim3 grid(LaunchUtils::GetBlockCount(materialEvalQueue.GetCapacity(), block.x));

		if (useNextEventEstimation)
			EvaluateDiffuseKernel<true><<<grid, block, 0, stream>>>(pathPool, materialEvalQueue, triangles, materials, pathDepthLimit, lightSampler, regenQueue, nextRayQueue);
		else
			EvaluateDiffuseKernel<false><<<grid, block, 0, stream>>>(pathPool, materialEvalQueue, triangles, materials, pathDepthLimit, lightSampler, regenQueue, nextRayQueue);
	}

	void EvaluatePhongMaterial(
		bool useNextEventEstimation,
		cudaStream_t stream,
		PathPoolView pathPool,
		MaterialEvalQueueView materialEvalQueue,
		Runtime::DeviceBuffer1DView<TriangleShading> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		LightSamplerView lightSampler,
		RegenQueueView regenQueue,
		Runtime::DeviceBuffer1DView<float4> accumulationBuffer,
		RayQueueView nextRayQueue)
	{
		dim3 block(LaunchUtils::MaterialEvalThreadsPerBlock);
		dim3 grid(LaunchUtils::GetBlockCount(materialEvalQueue.GetCapacity(), block.x));

		if (useNextEventEstimation)
			EvaluatePhongKernel<true><<<grid, block, 0, stream>>>(pathPool, materialEvalQueue, triangles, materials, pathDepthLimit, lightSampler, regenQueue, nextRayQueue);
		else
			EvaluatePhongKernel<false><<<grid, block, 0, stream>>>(pathPool, materialEvalQueue, triangles, materials, pathDepthLimit, lightSampler, regenQueue, nextRayQueue);
	}
	
	void EvaluateMirrorMaterial(
		bool useNextEventEstimation,
		cudaStream_t stream,
		PathPoolView pathPool,
		MaterialEvalQueueView materialEvalQueue,
		Runtime::DeviceBuffer1DView<TriangleShading> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		LightSamplerView lightSampler,
		RegenQueueView regenQueue,
		Runtime::DeviceBuffer1DView<float4> accumulationBuffer,
		RayQueueView nextRayQueue)
	{
		dim3 block(LaunchUtils::MaterialEvalThreadsPerBlock);
		dim3 grid(LaunchUtils::GetBlockCount(materialEvalQueue.GetCapacity(), block.x));
		
		EvaluateMirrorKernel<<<grid, block, 0, stream>>>(pathPool, materialEvalQueue, triangles, materials, pathDepthLimit, regenQueue, nextRayQueue);
	}
	

	void EvaluateGgxMaterial(
		bool useNextEventEstimation,
		cudaStream_t stream,
		PathPoolView pathPool,
		MaterialEvalQueueView materialEvalQueue,
		Runtime::DeviceBuffer1DView<TriangleShading> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		LightSamplerView lightSampler,
		RegenQueueView regenQueue,
		Runtime::DeviceBuffer1DView<float4> accumulationBuffer,
		RayQueueView nextRayQueue)
	{
		dim3 block(LaunchUtils::MaterialEvalThreadsPerBlock);
		dim3 grid(LaunchUtils::GetBlockCount(materialEvalQueue.GetCapacity(), block.x));
		
		if (useNextEventEstimation)
			EvaluateGgxKernel<true><<<grid, block, 0, stream>>>(pathPool, materialEvalQueue, triangles, materials, pathDepthLimit, lightSampler, regenQueue, nextRayQueue);
		else
			EvaluateGgxKernel<false><<<grid, block, 0, stream>>>(pathPool, materialEvalQueue, triangles, materials, pathDepthLimit, lightSampler, regenQueue, nextRayQueue);
	}
	
	void EvaluateEmissiveMaterial(
		bool useNextEventEstimation,
		cudaStream_t stream,
		PathPoolView pathPool,
		MaterialEvalQueueView materialEvalQueue,
		Runtime::DeviceBuffer1DView<TriangleShading> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		LightSamplerView lightSampler,
		RegenQueueView regenQueue,
		Runtime::DeviceBuffer1DView<float4> accumulationBuffer,
		RayQueueView nextRayQueue)
	{
		dim3 block(LaunchUtils::MaterialEvalThreadsPerBlock);
		dim3 grid(LaunchUtils::GetBlockCount(materialEvalQueue.GetCapacity(), block.x));
		
		if (useNextEventEstimation)
			EvaluateEmissiveKernel<true><<<grid, block, 0, stream>>>(pathPool, materialEvalQueue, triangles, materials, regenQueue, accumulationBuffer);
		else
			EvaluateEmissiveKernel<false><<<grid, block, 0, stream>>>(pathPool, materialEvalQueue, triangles, materials, regenQueue, accumulationBuffer);
	}
}