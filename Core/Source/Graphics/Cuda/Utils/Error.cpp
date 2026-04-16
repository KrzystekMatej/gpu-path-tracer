#include <Core/Graphics/Cuda/Utils/Error.hpp>

namespace Core::Graphics::Cuda::Utils
{
    std::string GetCudaErrorMessage(cudaError_t error)
    {
        const char* name = cudaGetErrorName(error);
        const char* message = cudaGetErrorString(error);

        if (name && message)
        {
            return std::string(name) + ": " + message;
        }

        if (message)
        {
            return std::string(message);
        }

        if (name)
        {
            return std::string(name);
        }

        return "Unknown CUDA error";
    }

    Core::Utils::Error MakeCudaError(const char* operation, cudaError_t error)
	{
		return Core::Utils::Error("{} failed: {}", operation, Utils::GetCudaErrorMessage(error));
	}
}