#include <Core/Graphics/Cuda/Resources/Texture.hpp>
#include <Core/Graphics/Cuda/Utils/Error.hpp>
#include <cuda_runtime.h>
#include <Core/Import/ImageUtils.hpp>

namespace Core::Graphics::Cuda::Resources
{
    namespace
    {
        using Core::Graphics::Common::ChannelLayout;
        using Core::Graphics::Common::ComponentType;

        std::expected<cudaChannelFormatDesc, Core::Utils::Error> CreateChannelDesc(const Import::Image& image)
        {
            switch (image.format.componentType)
            {
                case ComponentType::UInt8:
                {
                    switch (image.format.layout)
                    {
                        case ChannelLayout::R:
                            return cudaCreateChannelDesc<uchar1>();
                        case ChannelLayout::RGBA:
                            return cudaCreateChannelDesc<uchar4>();
                        case ChannelLayout::RGB:
                            break;
                    }
                    break;
                }

                case ComponentType::Float16:
                {
                    switch (image.format.layout)
                    {
                        case ChannelLayout::R:
                            return cudaCreateChannelDesc<ushort1>();
                        case ChannelLayout::RGBA:
                            return cudaCreateChannelDesc<ushort4>();
                        case ChannelLayout::RGB:
                            break;
                    }
                    break;
                }

                case ComponentType::Float32:
                {
                    switch (image.format.layout)
                    {
                        case ChannelLayout::R:
                            return cudaCreateChannelDesc<float1>();
                        case ChannelLayout::RGBA:
                            return cudaCreateChannelDesc<float4>();
                        case ChannelLayout::RGB:
                            break;
                    }
                    break;
                }
            }

            return std::unexpected(Core::Utils::Error("Unsupported CUDA texture format"));
        }

        cudaTextureDesc CreateTextureDesc(const Import::Image& image)
        {
            cudaTextureDesc textureDesc{};
            textureDesc.addressMode[0] = cudaAddressModeClamp;
            textureDesc.addressMode[1] = cudaAddressModeClamp;
            textureDesc.filterMode = cudaFilterModeLinear;
            textureDesc.normalizedCoords = 1;

            switch (image.format.componentType)
            {
                case ComponentType::UInt8:
                    textureDesc.readMode = cudaReadModeNormalizedFloat;
                    break;

                case ComponentType::Float16:
                case ComponentType::Float32:
                    textureDesc.readMode = cudaReadModeElementType;
                    break;
            }

            return textureDesc;
        }
    }
    Texture::~Texture()
    {
        if (m_CudaTexture)
        {
            cudaDestroyTextureObject(static_cast<cudaTextureObject_t>(m_CudaTexture));
            m_CudaTexture = 0;
        }

        if (m_CudaArray)
        {
            cudaFreeArray(reinterpret_cast<cudaArray_t>(m_CudaArray));
            m_CudaArray = nullptr;
        }
    }

    Texture::Texture(Texture&& other) noexcept
        : m_CudaArray(std::exchange(other.m_CudaArray, nullptr))
        , m_CudaTexture(std::exchange(other.m_CudaTexture, 0))
    {
    }

    Texture& Texture::operator=(Texture&& other) noexcept
    {
        if (this != &other)
        {
            if (m_CudaTexture)
                cudaDestroyTextureObject(static_cast<cudaTextureObject_t>(m_CudaTexture));

            if (m_CudaArray)
                cudaFreeArray(reinterpret_cast<cudaArray_t>(m_CudaArray));

            m_CudaArray = std::exchange(other.m_CudaArray, nullptr);
            m_CudaTexture = std::exchange(other.m_CudaTexture, 0);
        }

        return *this;
    }

    std::expected<Texture, Core::Utils::Error> Texture::Create2D(const Import::Image& image)
    {
        const Import::Image* sourceImage = &image;
		std::optional<Import::Image> convertedImage;

		if (image.format.layout == Graphics::Common::ChannelLayout::RGB)
		{
			convertedImage = std::move(Import::ConvertRgbToRgba(image));
            assert(convertedImage.has_value());
			sourceImage = &convertedImage.value();
		}

        auto channelDescResult = CreateChannelDesc(*sourceImage);
        if (!channelDescResult)
            return std::unexpected(Core::Utils::Error(std::move(channelDescResult).error()));

        Texture texture;

        cudaError_t error = cudaMallocArray(
            reinterpret_cast<cudaArray_t*>(&texture.m_CudaArray),
            &channelDescResult.value(),
            sourceImage->width,
            sourceImage->height);

        if (error != cudaSuccess)
        {
            return std::unexpected(Utils::MakeCudaError("cudaCreateTextureObject", error));
        }

        const size_t pitchBytes = static_cast<size_t>(sourceImage->width) * static_cast<size_t>(sourceImage->format.GetBytesPerPixel());

        error = cudaMemcpy2DToArray(
            reinterpret_cast<cudaArray_t>(texture.m_CudaArray),
            0,
            0,
            sourceImage->data.data(),
            pitchBytes,
            pitchBytes,
            sourceImage->height,
            cudaMemcpyHostToDevice);


        if (error != cudaSuccess)
        {
            const std::string cudaMessage = Utils::GetCudaErrorMessage(error);
            cudaFreeArray(reinterpret_cast<cudaArray_t>(texture.m_CudaArray));
            texture.m_CudaArray = nullptr;
            return std::unexpected(Utils::MakeCudaError("cudaMemcpy2DToArray", error));
        }

        cudaResourceDesc resourceDesc{};
        resourceDesc.resType = cudaResourceTypeArray;
        resourceDesc.res.array.array = reinterpret_cast<cudaArray_t>(texture.m_CudaArray);

        cudaTextureDesc textureDesc = CreateTextureDesc(*sourceImage);

        cudaTextureObject_t textureObject = 0;
        error = cudaCreateTextureObject(&textureObject, &resourceDesc, &textureDesc, nullptr);

        if (error != cudaSuccess)
        {
            const std::string cudaMessage = Utils::GetCudaErrorMessage(error);
            cudaFreeArray(reinterpret_cast<cudaArray_t>(texture.m_CudaArray));
            texture.m_CudaArray = nullptr;
            return std::unexpected(Utils::MakeCudaError("cudaCreateTextureObject", error));
        }

        texture.m_CudaTexture = static_cast<uint64_t>(textureObject);
        return texture;
    }
}
