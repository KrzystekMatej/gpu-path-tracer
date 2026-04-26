#pragma once
#include <cstddef>
#include <cstdint>
#include <cuda_runtime.h>
#include <Core/Graphics/Cuda/Memory/DeviceBuffer1DView.hpp>
#include <Core/Graphics/Cuda/Memory/DeviceBuffer2DView.hpp>
#include <Core/Graphics/Cuda/Memory/DeviceQueueView.hpp>
#include <Core/Graphics/Cuda/Memory/CounterView.hpp>
#include <Core/Graphics/Cuda/Resources/TextureView.hpp>

namespace Core::Graphics::Cuda::Memory
{
    template<typename T>
    __device__ __forceinline__ T& At(DeviceBuffer1DView<T> view, uint32_t idx)
    {
        return view.data[idx];
    }

    template<typename T>
    __device__ __forceinline__ T& At(DeviceBuffer2DView<T> view, uint32_t row, uint32_t col)
    {
		return *reinterpret_cast<T*>(reinterpret_cast<uint8_t*>(view.data) + row * view.pitchBytes + col * sizeof(T));
    }

    __device__ __forceinline__ uint32_t Get(CounterView view)
    {
        return *view.data;
    }

    __device__ __forceinline__ void Set(CounterView view, uint32_t value)
    {
        *view.data = value;
    }

    __device__ __forceinline__ uint32_t Increment(CounterView view)
    {
        return atomicAdd(view.data, 1u);
    }

    __device__ __forceinline__ uint32_t Add(CounterView view, uint32_t value)
    {
        return atomicAdd(view.data, value);
    }

    template<typename T>
    __device__ __forceinline__ T& At(DeviceQueueView<T> view, uint32_t idx)
    {
        return view.data[idx];
    }

    template<typename T>
    __device__ __forceinline__ bool Push(DeviceQueueView<T> view, const T& value)
    {
        const uint32_t idx = Increment(view.counter);

        if (idx >= view.capacity)
        {
            atomicSub(view.counter, 1u);
            return false;
        }

        view.data[idx] = value;
        return true;
    }

    template<typename T>
    __device__ __forceinline__ uint32_t PushUnchecked(DeviceQueueView<T> view, const T& value)
    {
        const uint32_t idx = Increment(view.counter);
        view.data[idx] = value;
        return idx;
    }

	template<typename T>
    __device__ __forceinline__ T Sample(TextureView<T> view, float u, float v)
    {
        return tex2D<T>(static_cast<cudaTextureObject_t>(view.texture), u, v);
	}
}