#include <Core/Graphics/Cuda/PathTracing/Kernels/LaunchUtils.cuh>
#include <Core/Graphics/Cuda/PathTracing/Kernels.hpp>
#include <Core/Graphics/Cuda/Utils/Math.hpp>
#include <Core/Graphics/Cuda/PathTracing/PathTracerDefaults.hpp>

namespace Core::Graphics::Cuda::Kernels
{
	__device__ __forceinline__ float3 GetGeometricNormal(const Triangle& triangle)
	{
		float3 v0 = triangle.vertices[0].position;
		float3 v1 = triangle.vertices[1].position;
		float3 v2 = triangle.vertices[2].position;

		float3 e1 = v1 - v0;
		float3 e2 = v2 - v0;

		return normalize(cross(e1, e2));
	}

	inline __forceinline__ __host__ __device__ Math::Frame GetShadingFrame(const Triangle& triangle, float4 normalMapSample, float b0, float b1, float b2)
	{
		float3 baseNormal = normalize(
			b0 * triangle.vertices[0].normal +
			b1 * triangle.vertices[1].normal +
			b2 * triangle.vertices[2].normal
		);

		float4 interpolatedTangent =
			b0 * triangle.vertices[0].tangent +
			b1 * triangle.vertices[1].tangent +
			b2 * triangle.vertices[2].tangent;

		float3 baseTangent = make_float3(
			interpolatedTangent.x,
			interpolatedTangent.y,
			interpolatedTangent.z
		);

		float tangentSign = interpolatedTangent.w < 0.0f ? -1.0f : 1.0f;
		Math::Frame baseFrame = Math::BuildFrame(baseNormal, baseTangent, tangentSign);

		float3 tangentSpaceNormal = normalize(make_float3(
			normalMapSample.x * 2.0f - 1.0f,
			normalMapSample.y * 2.0f - 1.0f,
			normalMapSample.z * 2.0f - 1.0f
		));

		float3 shadingNormal = normalize(
			tangentSpaceNormal.x * baseFrame.tangent +
			tangentSpaceNormal.y * baseFrame.bitangent +
			tangentSpaceNormal.z * baseFrame.normal
		);

		return Math::BuildFrame(shadingNormal, baseTangent, tangentSign);
	}

	__device__ __forceinline__ float3 SampleCosineWeightedHemisphere(float2 u)
	{
		float phi = 2.0f * CUDART_PI_F * u.x;
		float cosTheta = sqrtf(u.y);
		float sinTheta = sqrtf(1.0f - u.y);
		
		return make_float3(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta);
	}

	__device__ __forceinline__ float3 SamplePhongLobe(float3 reflected, float exponent, float2 u)
	{
		float phi = 2.0f * CUDART_PI_F * u.x;
		float cosTheta = powf(u.y, 1.0f / (exponent + 1.0f));
		float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);

		float3 sampled = make_float3(sinTheta * cosf(phi), sinTheta * sinf(phi), cosTheta);
		
		return Math::BuildFrame(reflected).FromLocal(sampled);
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

	__device__ __forceinline__ float Luminance(float3 color)
	{
		return 0.2126f * color.x + 0.7152f * color.y + 0.0722f * color.z;
	}
	
	__device__ __forceinline__ bool ApplyRussianRoulette(uint32_t depth, Contribution& contribution, float u)
	{
		if (depth < PathTracerDefaults::RussianRouletteStartDepth)
			return false;

		float alpha = Math::MaxComponent(make_float3(contribution.throughput));
		alpha = clamp(alpha, 0.0f, 1.0f);
		bool terminated = alpha < u;

		if (!terminated)
			contribution.throughput = contribution.throughput / alpha;
		
		return terminated;
	}

	__global__ void EvaluateNormalKernel(
		PathPoolView pathPool, 
		MaterialEvalQueueView materialEvalQueue, 
		Runtime::DeviceBuffer1DView<Triangle> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		RegenQueueView regenQueue,
		Runtime::DeviceBuffer1DView<float4> accumulationBuffer)
	{
		const uint32_t queueIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (queueIndex >= materialEvalQueue.GetSize()) return;
		
		const Path path = materialEvalQueue.GetPath(queueIndex);
		const HitData hitData = materialEvalQueue.GetHitData(queueIndex);
		const Material material = materials.At(hitData.material);
		
		Triangle triangle = triangles.At(hitData.triangle);
		float b0 = 1.0f - hitData.u - hitData.v;
		float b1 = hitData.u;
		float b2 = hitData.v;
		float2 uv =
			b0 * triangle.vertices[0].uv +
			b1 * triangle.vertices[1].uv +
			b2 * triangle.vertices[2].uv;
		
		Math::Frame shadingFrame = GetShadingFrame(triangle, material.normal.Sample(uv.x, uv.y), b0, b1, b2);

		const uint32_t pixelIndex = pathPool.pixels.At(path.index).index;
		
		float3 normal = shadingFrame.normal;
		Contribution contribution = pathPool.contributions.At(path.index);
		const float4 radiance = contribution.throughput * make_float4(normal.x * 0.5f + 0.5f, normal.y * 0.5f + 0.5f, normal.z * 0.5f + 0.5f, 1.0f);

		atomicAdd(&accumulationBuffer.At(pixelIndex).x, radiance.x);
		atomicAdd(&accumulationBuffer.At(pixelIndex).y, radiance.y);
		atomicAdd(&accumulationBuffer.At(pixelIndex).z, radiance.z);
		regenQueue.Push(path);
	}

	__global__ void EvaluateDiffuseKernel(
		PathPoolView pathPool, 
		MaterialEvalQueueView materialEvalQueue, 
		Runtime::DeviceBuffer1DView<Triangle> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		RegenQueueView regenQueue,
		RayQueueView nextRayQueue)
	{
		const uint32_t queueIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (queueIndex >= materialEvalQueue.GetSize()) return;
		
		Path path = materialEvalQueue.GetPath(queueIndex);
		Ray ray = materialEvalQueue.GetRay(queueIndex);
		const HitData hitData = materialEvalQueue.GetHitData(queueIndex);
		const Material material = materials.At(hitData.material);
		Triangle triangle = triangles.At(hitData.triangle);
        
        Random random = pathPool.randoms.At(path.index);
        float3 samples = random.NextFloat3();
        float2 directionSample = make_float2(samples.x, samples.y);
		float rrSample = samples.z; 
		pathPool.randoms.At(path.index) = random;

		float b0 = 1.0f - hitData.u - hitData.v;
		float b1 = hitData.u;
		float b2 = hitData.v;
		float2 uv =
			b0 * triangle.vertices[0].uv +
			b1 * triangle.vertices[1].uv +
			b2 * triangle.vertices[2].uv;

		Math::Frame shadingFrame = GetShadingFrame(triangle, material.normal.Sample(uv.x, uv.y), b0, b1, b2);
		float4 albedo = material.color.Sample(uv.x, uv.y);
		
		float3 omegaI = shadingFrame.FromLocal(SampleCosineWeightedHemisphere(directionSample));
																										  
		float3 geometricNormal = GetGeometricNormal(triangle);
		if (dot(ray.direction, geometricNormal) > 0.0f)
			geometricNormal = -geometricNormal;
		
		ray.origin = ray.origin + ray.direction * ray.tMax + geometricNormal * 1e-4f;
		ray.direction = omegaI;
		ray.tMin = PathTracerDefaults::MinT;
		ray.tMax = PathTracerDefaults::MaxT;
		Contribution contribution = pathPool.contributions.At(path.index);
		contribution.throughput *= albedo;
		
		PathFlags& pathFlags = pathPool.pathFlags.At(path.index);
		pathFlags.depth++;
		bool pathTerminated = pathFlags.depth >= pathDepthLimit || ApplyRussianRoulette(pathFlags.depth, contribution, rrSample);
		pathPool.contributions.At(path.index) = contribution;

		if (pathTerminated)
		{
			regenQueue.Push(path);
		}
		else
		{
			nextRayQueue.Push(path, ray);
		}
	}
	
	__global__ void EvaluateMirrorKernel(
		PathPoolView pathPool, 
		MaterialEvalQueueView materialEvalQueue, 
		Runtime::DeviceBuffer1DView<Triangle> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		RegenQueueView regenQueue,
		RayQueueView nextRayQueue)
	{
		const uint32_t queueIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (queueIndex >= materialEvalQueue.GetSize()) return;
		
		Path path = materialEvalQueue.GetPath(queueIndex);
		Ray ray = materialEvalQueue.GetRay(queueIndex);
		const HitData hitData = materialEvalQueue.GetHitData(queueIndex);
		const Material material = materials.At(hitData.material);
		Triangle triangle = triangles.At(hitData.triangle);

		Random random = pathPool.randoms.At(path.index);
		float rrSample = random.NextFloat();
		pathPool.randoms.At(path.index) = random;

		float b0 = 1.0f - hitData.u - hitData.v;
		float b1 = hitData.u;
		float b2 = hitData.v;
		float2 uv =
			b0 * triangle.vertices[0].uv +
			b1 * triangle.vertices[1].uv +
			b2 * triangle.vertices[2].uv;

		Math::Frame shadingFrame = GetShadingFrame(triangle, material.normal.Sample(uv.x, uv.y), b0, b1, b2);
		
		float3 omegaI = reflect(ray.direction, shadingFrame.normal);
		float4 reflectance = material.specular.Sample(uv.x, uv.y);

		float3 geometricNormal = GetGeometricNormal(triangle);
		if (dot(ray.direction, geometricNormal) > 0.0f)
			geometricNormal = -geometricNormal;

		ray.origin = ray.origin + ray.direction * ray.tMax + geometricNormal * 1e-4f;
		ray.direction = omegaI;
		ray.tMin = PathTracerDefaults::MinT;
		ray.tMax = PathTracerDefaults::MaxT;
		Contribution contribution = pathPool.contributions.At(path.index);
		contribution.throughput *= reflectance;
		
		PathFlags& pathFlags = pathPool.pathFlags.At(path.index);
		pathFlags.depth++;
		bool pathTerminated = pathFlags.depth >= pathDepthLimit || ApplyRussianRoulette(pathFlags.depth, contribution, rrSample);
		pathPool.contributions.At(path.index) = contribution;

		if (pathTerminated)
		{
			regenQueue.Push(path);
		}
		else
		{
			nextRayQueue.Push(path, ray);
		}
	}

	__global__ void EvaluatePhongKernel(
		PathPoolView pathPool, 
		MaterialEvalQueueView materialEvalQueue, 
		Runtime::DeviceBuffer1DView<Triangle> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		RegenQueueView regenQueue,
		RayQueueView nextRayQueue)
	{
		const uint32_t queueIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (queueIndex >= materialEvalQueue.GetSize()) return;
		
		Path path = materialEvalQueue.GetPath(queueIndex);
		Ray ray = materialEvalQueue.GetRay(queueIndex);
		const HitData hitData = materialEvalQueue.GetHitData(queueIndex);
		const Material material = materials.At(hitData.material);
		Triangle triangle = triangles.At(hitData.triangle);
        
		Random random = pathPool.randoms.At(path.index);
        float4 samples = random.NextFloat4();
		float2 directionSample = make_float2(samples.x, samples.y);
		float lobeSample = samples.z;
		float rrSample = samples.w;
		pathPool.randoms.At(path.index) = random;

		float b0 = 1.0f - hitData.u - hitData.v;
		float b1 = hitData.u;
		float b2 = hitData.v;

		float2 uv =
			b0 * triangle.vertices[0].uv +
			b1 * triangle.vertices[1].uv +
			b2 * triangle.vertices[2].uv;

		float3 geometricNormal = GetGeometricNormal(triangle);
		if (dot(ray.direction, geometricNormal) > 0.0f)
			geometricNormal = -geometricNormal;

		Math::Frame shadingFrame = GetShadingFrame(triangle, material.normal.Sample(uv.x, uv.y), b0, b1, b2);

		float4 albedo = material.color.Sample(uv.x, uv.y);
		float4 specular = material.specular.Sample(uv.x, uv.y);

		float shininess = material.shininess.Sample(uv.x, uv.y) * (MaterialDefaults::MaxShininess - MaterialDefaults::MinShininess) + MaterialDefaults::MinShininess;
		float diffuseWeight = Math::MaxComponent(make_float3(albedo));
		float specularWeight = Math::MaxComponent(make_float3(specular));
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

		float4 diffuseBrdf = albedo * Math::InvPi;
		float4 specularBrdf = specular * (shininess + 2.0f) * phongLobeOverTwoPi;

		ray.origin = ray.origin + ray.direction * ray.tMax + geometricNormal * 1e-4f;
		ray.direction = omegaI;
		ray.tMin = PathTracerDefaults::MinT;
		ray.tMax = PathTracerDefaults::MaxT;
		Contribution contribution = pathPool.contributions.At(path.index);
		contribution.throughput *= (diffuseBrdf + specularBrdf) * (cosThetaI / pdf);
		
		PathFlags& pathFlags = pathPool.pathFlags.At(path.index);
		pathFlags.depth++;
		bool pathTerminated = pathFlags.depth >= pathDepthLimit || ApplyRussianRoulette(pathFlags.depth, contribution, rrSample);
		pathPool.contributions.At(path.index) = contribution;

		if (pathTerminated)
		{
			regenQueue.Push(path);
		}
		else
		{
			nextRayQueue.Push(path, ray);
		}
	}
	

	__global__ void EvaluateGgxKernel(
		PathPoolView pathPool, 
		MaterialEvalQueueView materialEvalQueue, 
		Runtime::DeviceBuffer1DView<Triangle> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		uint32_t pathDepthLimit,
		RegenQueueView regenQueue,
		RayQueueView nextRayQueue)
	{
		const uint32_t queueIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (queueIndex >= materialEvalQueue.GetSize()) return;
		
		Path path = materialEvalQueue.GetPath(queueIndex);
		Ray ray = materialEvalQueue.GetRay(queueIndex);
		const HitData hitData = materialEvalQueue.GetHitData(queueIndex);
		const Material material = materials.At(hitData.material);
		Triangle triangle = triangles.At(hitData.triangle);

		Random random = pathPool.randoms.At(path.index);
		float4 samples = random.NextFloat4();
		float2 directionSample = make_float2(samples.x, samples.y);
		float lobeSample = samples.z;
		float rrSample = samples.w;
		pathPool.randoms.At(path.index) = random;

		float b0 = 1.0f - hitData.u - hitData.v;
		float b1 = hitData.u;
		float b2 = hitData.v;

		float2 uv =
			b0 * triangle.vertices[0].uv +
			b1 * triangle.vertices[1].uv +
			b2 * triangle.vertices[2].uv;

		float3 geometricNormal = GetGeometricNormal(triangle);
		if (dot(ray.direction, geometricNormal) > 0.0f)
			geometricNormal = -geometricNormal;
		
		Math::Frame shadingFrame = GetShadingFrame(triangle, material.normal.Sample(uv.x, uv.y), b0, b1, b2);
		float3 baseColor = make_float3(material.color.Sample(uv.x, uv.y));
		float4 rma = material.rma.Sample(uv.x, uv.y);

		float alpha = rma.x * rma.x;
		alpha = fmaxf(alpha, 0.001f); // in the future we should treat alpha=0 as a special case - effectively smooth surface
		float3 wo = shadingFrame.ToLocal(-ray.direction);

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
		
		ray.origin = ray.origin + ray.direction * ray.tMax + geometricNormal * 1e-4f;
		ray.direction = worldWi;
		ray.tMin = PathTracerDefaults::MinT;
		ray.tMax = PathTracerDefaults::MaxT;
		Contribution contribution = pathPool.contributions.At(path.index);
		contribution.throughput *= make_float4((diffuseBsdf + ggxBsdf) * (cosThetaI / pdf), 1.0f);
		
		PathFlags& pathFlags = pathPool.pathFlags.At(path.index);
		pathFlags.depth++;
		bool pathTerminated = pathFlags.depth >= pathDepthLimit || ApplyRussianRoulette(pathFlags.depth, contribution, rrSample);
		pathPool.contributions.At(path.index) = contribution;

		if (pathTerminated)
		{
			regenQueue.Push(path);
		}
		else
		{
			nextRayQueue.Push(path, ray);
		}
	}


	__global__ void EvaluateEmissiveKernel(
		PathPoolView pathPool, 
		MaterialEvalQueueView materialEvalQueue, 
		Runtime::DeviceBuffer1DView<Triangle> triangles,
		Runtime::DeviceBuffer1DView<Material> materials,
		RegenQueueView regenQueue,
		Runtime::DeviceBuffer1DView<float4> accumulationBuffer)
	{
		const uint32_t queueIndex = blockIdx.x * blockDim.x + threadIdx.x;
		if (queueIndex >= materialEvalQueue.GetSize()) return;
		
		Path path = materialEvalQueue.GetPath(queueIndex);
		const HitData hitData = materialEvalQueue.GetHitData(queueIndex);
		const Material material = materials.At(hitData.material);
		Triangle triangle = triangles.At(hitData.triangle);
		
		float b0 = 1.0f - hitData.u - hitData.v;
		float b1 = hitData.u;
		float b2 = hitData.v;
		float2 uv =
			b0 * triangle.vertices[0].uv +
			b1 * triangle.vertices[1].uv +
			b2 * triangle.vertices[2].uv;
		
		const float4 emission = material.emission.Sample(uv.x, uv.y);
		const uint32_t pixelIndex = pathPool.pixels.At(path.index).index;
		Contribution contribution = pathPool.contributions.At(path.index);
		const float4 radiance = contribution.throughput * emission;

		atomicAdd(&accumulationBuffer.At(pixelIndex).x, radiance.x);
		atomicAdd(&accumulationBuffer.At(pixelIndex).y, radiance.y);
		atomicAdd(&accumulationBuffer.At(pixelIndex).z, radiance.z);
		regenQueue.Push(path);
	}

	void EvaluateNormalMaterial(
		cudaStream_t stream,
		PathPoolView pathPool,
		MaterialEvalQueueView materialEvalQueue,
		Runtime::DeviceBuffer1DView<Triangle> triangles,
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
		Runtime::DeviceBuffer1DView<Triangle> triangles,
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
		Runtime::DeviceBuffer1DView<Triangle> triangles,
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
		Runtime::DeviceBuffer1DView<Triangle> triangles,
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
		Runtime::DeviceBuffer1DView<Triangle> triangles,
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
		Runtime::DeviceBuffer1DView<Triangle> triangles,
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