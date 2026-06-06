#pragma once
#include <cstddef>
#include <cuda_runtime.h>
#include "CounterView.hpp"

namespace Core::Graphics::Cuda::Runtime
{
    template<typename T>
    class DeviceQueueView
    {
    public:
        DeviceQueueView() = default;
        DeviceQueueView(T* data, uint32_t capacity, CounterView<uint32_t> counter)
            : m_Data(data), m_Capacity(capacity), m_Counter(counter) {}
        
        __device__ __forceinline__ T* GetData() const
        {
            return m_Data;
        }
    
        __device__ __forceinline__ uint32_t GetSize() const
        {
            return m_Counter.Get();
        }

        __device__ __forceinline__ void SetSize(uint32_t size) const
        {
            m_Counter.Set(size);
        }

        __host__ __device__ __forceinline__ uint32_t GetCapacity() const
        {
            return m_Capacity;
        }

        __device__ __forceinline__ bool IsEmpty() const
        {
            return m_Counter.Get() == 0;
        }

        __device__ __forceinline__ bool IsFull() const
        {
            return m_Counter.Get() >= m_Capacity;
        }

        __device__ __forceinline__ void Clear() const
        {
            m_Counter.Set(0);
        }

        __device__ __forceinline__ T& At(uint32_t index)
        {
            return m_Data[index];
        }

        __device__ __forceinline__ const T& At(uint32_t index) const
        {
            return m_Data[index];
        }

        __device__ __forceinline__ bool PushChecked(const T& value)
        {
            const uint32_t index = m_Counter.Increment();

            if (index >= m_Capacity)
            {
                m_Counter.Decrement();
                return false;
            }

            m_Data[index] = value;
            return true;
        }

        __device__ __forceinline__ uint32_t Push(const T& value)
        {
            const uint32_t index = m_Counter.Increment();
            m_Data[index] = value;
            return index;
        }
    private:
        T* __restrict__ m_Data = nullptr;
        uint32_t m_Capacity = 0;
        CounterView<uint32_t> m_Counter;
    };
}