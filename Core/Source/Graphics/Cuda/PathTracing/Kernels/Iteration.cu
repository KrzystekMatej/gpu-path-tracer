
#include <Core/Graphics/Cuda/PathTracing/Kernels/LaunchUtils.cuh>
#include <Core/Graphics/Cuda/PathTracing/Kernels/Launchers.hpp>

namespace Core::Graphics::Cuda::Kernels
{
	__global__ void PrepareIterationKernel(
		RayQueueView currentRayQueue,
		RayQueueView nextRayQueue,
		RegenQueueView regenQueue,
		MaterialEvalQueueViewsProvider materialQueueProvider,
		uint32_t nextRayCount)
	{
		currentRayQueue.Clear();
		nextRayQueue.SetSize(nextRayCount);
		regenQueue.Clear();
		for (uint32_t i = 0; i < static_cast<uint32_t>(GlobalShadingModel::Count); i++)
		{
			materialQueueProvider.At(i).Clear();
		}
	}
    
	void PrepareIteration(
		cudaStream_t stream,
		RayQueueView currentRayQueue,
		RayQueueView nextRayQueue,
		RegenQueueView regenQueue,
		MaterialEvalQueueViewsProvider materialQueueProvider,
		uint32_t nextRayCount)
	{
		PrepareIterationKernel<<<1, 1, 0, stream>>>(currentRayQueue, nextRayQueue, regenQueue, materialQueueProvider, nextRayCount);
	}
}