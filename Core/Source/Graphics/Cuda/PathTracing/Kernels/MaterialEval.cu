#include <Core/Graphics/Cuda/PathTracing/Kernels/LaunchUtils.cuh>
#include <Core/Graphics/Cuda/PathTracing/Kernels/Launchers.hpp>
#include <Core/Graphics/Cuda/Utils/Math.hpp>
#include <Core/Graphics/Cuda/PathTracing/PathTracerDefaults.hpp>

namespace Core::Graphics::Cuda::Kernels
{
	struct PartiallyResolvedHit
	{
		TriangleShading triangle;
		float2 uv;
	};

	struct ResolvedHit
	{
		TriangleShading triangle;
		float2 uv;
		float3 normal;
		float4 tangent;
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
		PartiallyResolvedHit hit;
		const HitData hitData = materialEvalQueue.GetHitData(queueIndex);
		hit.triangle = triangles.At(hitData.triangle);

		float b0 = 1.0f - hitData.u - hitData.v;
		float b1 = hitData.u;
		float b2 = hitData.v;

		hit.uv =
			b0 * hit.triangle.vertices[0].uv +
			b1 * hit.triangle.vertices[1].uv +
			b2 * hit.triangle.vertices[2].uv;

		return hit;
	}
	
	__device__ __forceinline__ ResolvedHit LoadResolvedHit(
		MaterialEvalQueueView materialEvalQueue, 
		uint32_t queueIndex, 
		Runtime::DeviceBuffer1DView<TriangleShading> triangles)
	{
		ResolvedHit hit;
		const HitData hitData = materialEvalQueue.GetHitData(queueIndex);
		hit.triangle = triangles.At(hitData.triangle);

		float b0 = 1.0f - hitData.u - hitData.v;
		float b1 = hitData.u;
		float b2 = hitData.v;

		hit.uv =
			b0 * hit.triangle.vertices[0].uv +
			b1 * hit.triangle.vertices[1].uv +
			b2 * hit.triangle.vertices[2].uv;

		hit.normal =
			b0 * make_float3(hit.triangle.vertices[0].normal) +
			b1 * make_float3(hit.triangle.vertices[1].normal) +
			b2 * make_float3(hit.triangle.vertices[2].normal);

		hit.tangent =
			b0 * hit.triangle.vertices[0].tangent +
			b1 * hit.triangle.vertices[1].tangent +
			b2 * hit.triangle.vertices[2].tangent;

		return hit;
	}

	__device__ __forceinline__ float3 LoadTangentSpaceNormal(TextureView<float4> normalMap, float2 uv)
	{
		float3 normalMapSample = make_float3(normalMap.Sample(uv.x, uv.y));
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

	__device__ __forceinline__ PathContinuation ComputePathContinuation(const Ray& ray, uint32_t pathDepthLimit, float3 scatterWeight, float rrSample)
	{
		uint32_t nextDepth = ray.depth + 1;
		if (nextDepth >= pathDepthLimit)
			return { true, nextDepth, make_float3(0.0f) };

		float3 nextThroughput = ray.throughput * scatterWeight;
		
		if (nextDepth < PathTracerDefaults::RussianRouletteStartDepth)
			return { false, nextDepth, nextThroughput };

		float survivalProbability = clamp(Math::MaxComponent(nextThroughput), 0.0f, 1.0f);

		if (rrSample > survivalProbability)
			return { true, nextDepth, make_float3(0.0f) };

		return { false, nextDepth, nextThroughput / survivalProbability };
	}

	__device__ __forceinline__ Ray SpawnScatteredRay(const Ray& ray, float3 nextDirection, float3 geometricNormal, const PathContinuation& continuation)
	{
		Ray spawnedRay;
		spawnedRay.origin = ray.origin + ray.direction * ray.tMax + geometricNormal * 1e-4f;
		spawnedRay.direction = nextDirection;
		spawnedRay.tMin = PathTracerDefaults::MinT;
		spawnedRay.tMax = PathTracerDefaults::MaxT;
		spawnedRay.throughput = continuation.throughput;
		spawnedRay.depth = continuation.depth;
		return spawnedRay;
	}
	
	__device__ __forceinline__ void AddRadiance(PathPoolView pathPool, Path path, float3 radiance, Runtime::DeviceBuffer1DView<float4> accumulationBuffer)
	{
		const Pixel pixel = pathPool.pixels.At(path.index);
		atomicAdd(&accumulationBuffer.At(pixel.index).x, radiance.x);
		atomicAdd(&accumulationBuffer.At(pixel.index).y, radiance.y);
		atomicAdd(&accumulationBuffer.At(pixel.index).z, radiance.z);
	}
	

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
		
		const Path path = materialEvalQueue.GetPath(queueIndex);
		const float3 throughput = materialEvalQueue.GetThroughput(queueIndex);
		const ResolvedHit hit = LoadResolvedHit(materialEvalQueue, queueIndex, triangles);
		
		// shading normal can be used instead of visual normal to see the effect of normal mapping in the debug view
		// const Material material = materials.At(hit.triangle.material);
		// float3 normal = GetShadingFrame(hit, material.normal).normal;
		float3 normal = hit.normal;
		AddRadiance(pathPool, path, throughput * (normal * 0.5f + 0.5f), accumulationBuffer);
		regenQueue.Push(path);
	}
	
	__device__ __forceinline__ float3 SampleCosineWeightedHemisphere(float2 u)
	{
		float phi = 2.0f * CUDART_PI_F * u.x;
		float cosTheta = sqrtf(u.y);
		float sinTheta = sqrtf(1.0f - u.y);
		
		return make_float3(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta);
	}


	__global__ void EvaluateDiffuseKernel(
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
		
		const Path path = materialEvalQueue.GetPath(queueIndex);
		const Ray ray = materialEvalQueue.GetRay(queueIndex);
		const ResolvedHit hit = LoadResolvedHit(materialEvalQueue, queueIndex, triangles);

		const Material material = materials.At(hit.triangle.material);
		const float3 albedo = make_float3(material.color.Sample(hit.uv.x, hit.uv.y));
		
		const float3 geometricNormal = FixGeometricNormal(hit.triangle.geometricNormal, ray.direction);
		const Math::Frame shadingFrame = GetShadingFrame(hit, material.normal);
        
        Random random = pathPool.randoms.At(path.index);
        const float3 samples = random.NextFloat3();
        const float2 directionSample = make_float2(samples.x, samples.y);
		const float rrSample = samples.z; 
		pathPool.randoms.At(path.index) = random;

		
		const float3 omegaI = shadingFrame.FromLocal(SampleCosineWeightedHemisphere(directionSample));
		
		const PathContinuation continuation = ComputePathContinuation(ray, pathDepthLimit, albedo, rrSample);

		if (continuation.terminated)
		{
			regenQueue.Push(path);
		}
		else
		{
			const Ray scatteredRay = SpawnScatteredRay(ray, omegaI, geometricNormal, continuation);
			nextRayQueue.Push(path, scatteredRay);
		}
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
		
		const Path path = materialEvalQueue.GetPath(queueIndex);
		const Ray ray = materialEvalQueue.GetRay(queueIndex);
		const ResolvedHit hit = LoadResolvedHit(materialEvalQueue, queueIndex, triangles);

		const Material material = materials.At(hit.triangle.material);
		const float3 reflectance = make_float3(material.specular.Sample(hit.uv.x, hit.uv.y));

		const float3 geometricNormal = FixGeometricNormal(hit.triangle.geometricNormal, ray.direction);
		const Math::Frame shadingFrame = GetShadingFrame(hit, material.normal);
		
		Random random = pathPool.randoms.At(path.index);
		const float rrSample = random.NextFloat();
		pathPool.randoms.At(path.index) = random;

		const float3 omegaI = reflect(ray.direction, shadingFrame.normal);

		const PathContinuation continuation = ComputePathContinuation(ray, pathDepthLimit, reflectance, rrSample);

		if (continuation.terminated)
		{
			regenQueue.Push(path);
		}
		else
		{
			const Ray scatteredRay = SpawnScatteredRay(ray, omegaI, geometricNormal, continuation);
			nextRayQueue.Push(path, scatteredRay);
		}
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

	__global__ void EvaluatePhongKernel(
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
		
		const Path path = materialEvalQueue.GetPath(queueIndex);
		const Ray ray = materialEvalQueue.GetRay(queueIndex);
		const ResolvedHit hit = LoadResolvedHit(materialEvalQueue, queueIndex, triangles);

		const Material material = materials.At(hit.triangle.material);
		const float3 albedo = make_float3(material.color.Sample(hit.uv.x, hit.uv.y));
		const float3 specular = make_float3(material.specular.Sample(hit.uv.x, hit.uv.y));
		const float shininess = LoadShininess(material.shininess, hit.uv);

		const float3 geometricNormal = FixGeometricNormal(hit.triangle.geometricNormal, ray.direction);
		const Math::Frame shadingFrame = GetShadingFrame(hit, material.normal);

		Random random = pathPool.randoms.At(path.index);
        const float4 samples = random.NextFloat4();
		const float2 directionSample = make_float2(samples.x, samples.y);
		const float lobeSample = samples.z;
		const float rrSample = samples.w;
		pathPool.randoms.At(path.index) = random;

		float diffuseWeight = Luminance(albedo);
		float specularWeight = Luminance(specular);
		float weightSum = diffuseWeight + specularWeight;

		if (weightSum <= 0.0f)
		{
			regenQueue.Push(path);
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
			regenQueue.Push(path);
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
			regenQueue.Push(path);
			return;
		}

		float3 diffuseBrdf = albedo * Math::InvPi;
		float3 specularBrdf = specular * (shininess + 2.0f) * phongLobeOverTwoPi;

		const PathContinuation continuation = ComputePathContinuation(ray, pathDepthLimit, (diffuseBrdf + specularBrdf) * (cosThetaI / pdf), rrSample);

		if (continuation.terminated)
		{
			regenQueue.Push(path);
		}
		else
		{
			const Ray scatteredRay = SpawnScatteredRay(ray, omegaI, geometricNormal, continuation);
			nextRayQueue.Push(path, scatteredRay);
		}
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

	__global__ void EvaluateGgxKernel(
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
		
		const Path path = materialEvalQueue.GetPath(queueIndex);
		const Ray ray = materialEvalQueue.GetRay(queueIndex);
		const ResolvedHit hit = LoadResolvedHit(materialEvalQueue, queueIndex, triangles);

		const Material material = materials.At(hit.triangle.material);
		const float3 baseColor = make_float3(material.color.Sample(hit.uv.x, hit.uv.y));
		const float3 rma = make_float3(material.rma.Sample(hit.uv.x, hit.uv.y));

		const float3 geometricNormal = FixGeometricNormal(hit.triangle.geometricNormal, ray.direction);
		const Math::Frame shadingFrame = GetShadingFrame(hit, material.normal);

		Random random = pathPool.randoms.At(path.index);
		const float4 samples = random.NextFloat4();
		const float2 directionSample = make_float2(samples.x, samples.y);
		const float lobeSample = samples.z;
		const float rrSample = samples.w;
		pathPool.randoms.At(path.index) = random;

		float alpha = rma.x * rma.x;
		alpha = fmaxf(alpha, 0.001f); // in the future we should treat alpha=0 as a special case - effectively smooth surface
		float3 wo = shadingFrame.ToLocal(make_float3(0.0f) - ray.direction);

		if (wo.z <= 0.0f)
		{
			regenQueue.Push(path);
			return;
		}

		float dielectricF0 = Math::Sqr((ray.ior - material.ior) / (ray.ior + material.ior));
		float3 f0 = lerp(make_float3(dielectricF0), baseColor, rma.y);

		float3 diffuseColor = baseColor * (1.0f - rma.y);

		float diffuseWeight = Luminance(diffuseColor);
		float specularWeight = Luminance(f0);

		float weightSum = diffuseWeight + specularWeight;
		if (weightSum <= 1e-6f)
		{
			regenQueue.Push(path);
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
			regenQueue.Push(path);
			return;
		}
		
		float3 wm = normalize(wi + wo);
		
		if (wm.z <= 0.0f)
		{
			regenQueue.Push(path);
			return;
		}

		float woDotWm = dot(wo, wm);
		
		if (woDotWm <= 1e-6f)
		{
			regenQueue.Push(path);
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
			regenQueue.Push(path);
			return;
		}
		
		float3 worldWi = shadingFrame.FromLocal(wi);
		if (dot(worldWi, geometricNormal) <= 0.0f)
		{
			regenQueue.Push(path);
			return;
		}
		
		const PathContinuation continuation = ComputePathContinuation(ray, pathDepthLimit, (diffuseBsdf + ggxBsdf) * (cosThetaI / pdf), rrSample);
		
		if (continuation.terminated)
		{
			regenQueue.Push(path);
		}
		else
		{
			const Ray scatteredRay = SpawnScatteredRay(ray, worldWi, geometricNormal, continuation);
			nextRayQueue.Push(path, scatteredRay);
		}
	}


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
		
		const Path path = materialEvalQueue.GetPath(queueIndex);
		const float3 throughput = materialEvalQueue.GetThroughput(queueIndex);
		const PartiallyResolvedHit hit = LoadPartiallyResolvedHit(materialEvalQueue, queueIndex, triangles);
		const Material material = materials.At(hit.triangle.material);
		
		const float3 emission = make_float3(material.emission.Sample(hit.uv.x, hit.uv.y));
		AddRadiance(pathPool, path, throughput * emission, accumulationBuffer);
		regenQueue.Push(path);
	}

	void EvaluateNormalMaterial(
		cudaStream_t stream,
		PathPoolView pathPool,
		MaterialEvalQueueView materialEvalQueue,
		Runtime::DeviceBuffer1DView<TriangleShading> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		RegenQueueView regenQueue,
		Runtime::DeviceBuffer1DView<float4> accumulationBuffer,
		RayQueueView nextRayQueue)
	{
		dim3 block(LaunchUtils::MaterialEvalThreadsPerBlock);
		dim3 grid(LaunchUtils::GetBlockCount(materialEvalQueue.GetCapacity(), block.x));
		EvaluateNormalKernel<<<grid, block, 0, stream>>>(pathPool, materialEvalQueue, triangles, materials, regenQueue, accumulationBuffer);
	}

	void EvaluateDiffuseMaterial(
		cudaStream_t stream,
		PathPoolView pathPool,
		MaterialEvalQueueView materialEvalQueue,
		Runtime::DeviceBuffer1DView<TriangleShading> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		RegenQueueView regenQueue,
		Runtime::DeviceBuffer1DView<float4> accumulationBuffer,
		RayQueueView nextRayQueue)
	{
		dim3 block(LaunchUtils::MaterialEvalThreadsPerBlock);
		dim3 grid(LaunchUtils::GetBlockCount(materialEvalQueue.GetCapacity(), block.x));
		EvaluateDiffuseKernel<<<grid, block, 0, stream>>>(pathPool, materialEvalQueue, triangles, materials, pathDepthLimit, regenQueue, nextRayQueue);
	}

	void EvaluatePhongMaterial(
		cudaStream_t stream,
		PathPoolView pathPool,
		MaterialEvalQueueView materialEvalQueue,
		Runtime::DeviceBuffer1DView<TriangleShading> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		RegenQueueView regenQueue,
		Runtime::DeviceBuffer1DView<float4> accumulationBuffer,
		RayQueueView nextRayQueue)
	{
		dim3 block(LaunchUtils::MaterialEvalThreadsPerBlock);
		dim3 grid(LaunchUtils::GetBlockCount(materialEvalQueue.GetCapacity(), block.x));
		EvaluatePhongKernel<<<grid, block, 0, stream>>>(pathPool, materialEvalQueue, triangles, materials, pathDepthLimit, regenQueue, nextRayQueue);
	}
	
	void EvaluateMirrorMaterial(
		cudaStream_t stream,
		PathPoolView pathPool,
		MaterialEvalQueueView materialEvalQueue,
		Runtime::DeviceBuffer1DView<TriangleShading> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		RegenQueueView regenQueue,
		Runtime::DeviceBuffer1DView<float4> accumulationBuffer,
		RayQueueView nextRayQueue)
	{
		dim3 block(LaunchUtils::MaterialEvalThreadsPerBlock);
		dim3 grid(LaunchUtils::GetBlockCount(materialEvalQueue.GetCapacity(), block.x));
		EvaluateMirrorKernel<<<grid, block, 0, stream>>>(pathPool, materialEvalQueue, triangles, materials, pathDepthLimit, regenQueue, nextRayQueue);
	}
	

	void EvaluateGgxMaterial(
		cudaStream_t stream,
		PathPoolView pathPool,
		MaterialEvalQueueView materialEvalQueue,
		Runtime::DeviceBuffer1DView<TriangleShading> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		RegenQueueView regenQueue,
		Runtime::DeviceBuffer1DView<float4> accumulationBuffer,
		RayQueueView nextRayQueue)
	{
		dim3 block(LaunchUtils::MaterialEvalThreadsPerBlock);
		dim3 grid(LaunchUtils::GetBlockCount(materialEvalQueue.GetCapacity(), block.x));
		EvaluateGgxKernel<<<grid, block, 0, stream>>>(pathPool, materialEvalQueue, triangles, materials, pathDepthLimit, regenQueue, nextRayQueue);
	}
	
	void EvaluateEmissiveMaterial(
		cudaStream_t stream,
		PathPoolView pathPool,
		MaterialEvalQueueView materialEvalQueue,
		Runtime::DeviceBuffer1DView<TriangleShading> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		RegenQueueView regenQueue,
		Runtime::DeviceBuffer1DView<float4> accumulationBuffer,
		RayQueueView nextRayQueue)
	{
		dim3 block(LaunchUtils::MaterialEvalThreadsPerBlock);
		dim3 grid(LaunchUtils::GetBlockCount(materialEvalQueue.GetCapacity(), block.x));
		EvaluateEmissiveKernel<<<grid, block, 0, stream>>>(pathPool, materialEvalQueue, triangles, materials, regenQueue, accumulationBuffer);
	}
}