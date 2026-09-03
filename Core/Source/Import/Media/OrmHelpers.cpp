#include <Core/Import/Media/OrmHelpers.hpp>
#include <Core/Import/Buffers/BufferUtils.hpp>
#include <Core/Utils/NumberTypes.hpp>


namespace Core::Import::Media
{
    std::expected<void, Utils::Error> ConvertRmoToOrm(Image& rmo)
    {
        assert(rmo.format.colorSpace == Graphics::ColorSpace::Linear && "RMO texture must be in linear color space");
        if (rmo.format.layout != Graphics::ChannelLayout::RGB && rmo.format.layout != Graphics::ChannelLayout::RGBA)
        {
            return std::unexpected(Utils::Error("RMO texture format must be RGB or RGBA"));
        }

        std::array<uint8_t, Graphics::ToComponentSize(Graphics::GetBiggestComponentType()) * 3> tempPixel;

        const uint32_t componentSize = rmo.format.GetComponentSize();
        const uint32_t bytesPerPixel = rmo.format.GetBytesPerPixel();

        uint8_t* sourcePtr = rmo.data.data();
        uint8_t* tempPtr = tempPixel.data();
        
        for (size_t offset = 0; offset < rmo.data.size(); offset += bytesPerPixel)
        {
			std::memcpy(tempPtr, sourcePtr + 2 * componentSize, componentSize);
            std::memcpy(tempPtr + componentSize, sourcePtr, componentSize * 2);
            
            std::memcpy(sourcePtr, tempPtr, componentSize * 3);

            sourcePtr += bytesPerPixel;
        }
        
        return {};
    }

    void WriteFallback(uint8_t* destination, float fallbackValue, Graphics::ComponentType componentType)
    {
        switch (componentType)
        {
            case Graphics::ComponentType::UInt8:
                *destination = Utils::NumberTypes::NormalizedFloatToU8(fallbackValue);
                break;
            case Graphics::ComponentType::Float16:
                *reinterpret_cast<uint16_t*>(destination) = Utils::NumberTypes::Float32ToFloat16(fallbackValue);
                break;
            case Graphics::ComponentType::Float32:
                *reinterpret_cast<float*>(destination) = fallbackValue;
                break;
        }
    }
    
    std::expected<Image, Utils::Error> PackOrmTexture(const std::optional<Image>& occlusion, float fallbackOcclusion, const Image& rm)
    {
        assert(rm.format.colorSpace == Graphics::ColorSpace::Linear && "RM texture must be in linear color space");
        if (rm.format.layout != Graphics::ChannelLayout::RG && rm.format.layout != Graphics::ChannelLayout::RGB && rm.format.layout != Graphics::ChannelLayout::RGBA)
        {
            return std::unexpected(Utils::Error("RM texture format must be RG, RGB, or RGBA"));
        }
        
        if (occlusion.has_value()) 
        {
            if (occlusion->width != rm.width || occlusion->height != rm.height)
                return std::unexpected(Utils::Error("Occlusion texture dimensions must match RM texture dimensions"));
            
            if (occlusion->format.componentType != rm.format.componentType)
                return std::unexpected(Utils::Error("Occlusion texture component type must match RM texture component type"));
            
            assert(occlusion->format.colorSpace == Graphics::ColorSpace::Linear && "Occlusion texture must be in linear color space");
        }

        Image orm;
        orm.format.layout = Graphics::ChannelLayout::RGB;
        orm.format.componentType = rm.format.componentType;
        orm.format.colorSpace = rm.format.colorSpace;
        orm.width = rm.width;
        orm.height = rm.height;
        orm.data.resize(orm.width * orm.height * orm.format.GetBytesPerPixel());
        
        const uint32_t componentSize = orm.format.GetComponentSize();
        const uint32_t occlusionBytesPerPixel = occlusion.has_value() ? occlusion->format.GetBytesPerPixel() : 0;
        const uint32_t rmBytesPerPixel = rm.format.GetBytesPerPixel();
        const uint32_t ormBytesPerPixel = orm.format.GetBytesPerPixel();
        
        const uint8_t* occlusionPtr = occlusion.has_value() ? occlusion->data.data() : nullptr;
        const uint8_t* rmPtr = rm.data.data();
        uint8_t* ormPtr = orm.data.data();
        
        for (size_t offset = 0; offset < orm.data.size(); offset += ormBytesPerPixel)
        {
            if (occlusion.has_value())
            {
                std::memcpy(ormPtr, occlusionPtr, componentSize);
                occlusionPtr += occlusionBytesPerPixel;
            }
            else
            {
                WriteFallback(ormPtr, fallbackOcclusion, orm.format.componentType);
            }
            
            if (rm.format.layout == Graphics::ChannelLayout::RG)
            {
                std::memcpy(ormPtr + componentSize, rmPtr, componentSize * 2);
            }
            else
            {
                // Roughness and metallic components are conventionally stored in the G and B channels in case of RGB or RGBA textures
                std::memcpy(ormPtr + componentSize, rmPtr + componentSize, componentSize * 2);
            }
            
            rmPtr += rmBytesPerPixel;
            ormPtr += ormBytesPerPixel;
        }
        
        

        return orm;
    }
    
    Image GetTemplateOrmImage(const std::optional<Image>& occlusion, const std::optional<Image>& roughness, const std::optional<Image>& metallic)
    {
        Image orm;
        orm.format.layout = Graphics::ChannelLayout::RGB;
        orm.format.colorSpace = Graphics::ColorSpace::Linear;
        
        if (occlusion.has_value())
        {
            orm.format.componentType = occlusion->format.componentType;
            orm.width = occlusion->width;
            orm.height = occlusion->height;
        }
        else if (roughness.has_value())
        {
            orm.format.componentType = roughness->format.componentType;
            orm.width = roughness->width;
            orm.height = roughness->height;
        }
        else if (metallic.has_value())
        {
            orm.format.componentType = metallic->format.componentType;
            orm.width = metallic->width;
            orm.height = metallic->height;
        }
        else
        {
            orm.format.componentType = Graphics::ComponentType::UInt8;
            orm.width = 1;
            orm.height = 1;
        }

        return orm;
    }
    

    std::expected<void, Utils::Error> CheckTextureAgainstTemplate(const Image& texture, const Image& templateImage, std::string_view textureName)
    {
        if (texture.width != templateImage.width || texture.height != templateImage.height)
            return std::unexpected(Utils::Error("Dimensions of {} texture must match packed ORM texture dimensions", textureName));

        if (texture.format.componentType != templateImage.format.componentType)
            return std::unexpected(Utils::Error("Component type of {} texture must match packed ORM texture component type", textureName));

        assert(texture.format.GetChannelCount() >= 1 && "Packed ORM texture must have at least one channel");
        assert(templateImage.format.colorSpace == Graphics::ColorSpace::Linear && "Packed ORM texture must be in linear color space");
        return {};
    }
    
    std::expected<Image, Utils::Error> PackOrmTexture(
        const std::optional<Image>& occlusion,
        float fallbackOcclusion, 
        const std::optional<Image>& roughness,
        float fallbackRoughness, 
        const std::optional<Image>& metallic, 
        float fallbackMetallic)
    {
        Image orm = GetTemplateOrmImage(occlusion, roughness, metallic);
        
        if (occlusion.has_value())
        {
            CORE_TRY_DISCARD_CONTEXT(CheckTextureAgainstTemplate(occlusion.value(), orm, "Occlusion"), "Failed to pack ORM texture");
        }
        
        if (roughness.has_value())
        {
            CORE_TRY_DISCARD_CONTEXT(CheckTextureAgainstTemplate(roughness.value(), orm, "Roughness"), "Failed to pack ORM texture");
        }

        if (metallic.has_value())
        {
            CORE_TRY_DISCARD_CONTEXT(CheckTextureAgainstTemplate(metallic.value(), orm, "Metallic"), "Failed to pack ORM texture");
        }
        
        orm.data.resize(orm.width * orm.height * orm.format.GetBytesPerPixel());
        
        const uint32_t componentSize = orm.format.GetComponentSize();
        const uint32_t occlusionBytesPerPixel = occlusion.has_value() ? occlusion->format.GetBytesPerPixel() : 0;
        const uint32_t roughnessBytesPerPixel = roughness.has_value() ? roughness->format.GetBytesPerPixel() : 0;
        const uint32_t metallicBytesPerPixel = metallic.has_value() ? metallic->format.GetBytesPerPixel() : 0;
        const uint32_t ormBytesPerPixel = orm.format.GetBytesPerPixel();

        const uint8_t* occlusionPtr = occlusion.has_value() ? occlusion->data.data() : nullptr;
        const uint8_t* roughnessPtr = roughness.has_value() ? roughness->data.data() : nullptr;
        const uint8_t* metallicPtr = metallic.has_value() ? metallic->data.data() : nullptr;
        uint8_t* ormPtr = orm.data.data();

        for (size_t offset = 0; offset < orm.data.size(); offset += ormBytesPerPixel)
        {
            if (occlusion.has_value())
            {
                std::memcpy(ormPtr, occlusionPtr, componentSize);
                occlusionPtr += occlusionBytesPerPixel;
            }
            else
            {
                WriteFallback(ormPtr, fallbackOcclusion, orm.format.componentType);
            }

            if (roughness.has_value())
            {
                std::memcpy(ormPtr + componentSize, roughnessPtr, componentSize);
                roughnessPtr += roughnessBytesPerPixel;
            }
            else
            {
                WriteFallback(ormPtr + componentSize, fallbackRoughness, orm.format.componentType);
            }

            if (metallic.has_value())
            {
                std::memcpy(ormPtr + 2 * componentSize, metallicPtr, componentSize);
                metallicPtr += metallicBytesPerPixel;
            }
            else
            {
                WriteFallback(ormPtr + 2 * componentSize, fallbackMetallic, orm.format.componentType);
            }

            ormPtr += ormBytesPerPixel;
        }


        return orm;
    }

    Image ExtractRmTexture(const Image& orm)
    {
        Image rm;
        rm.format.layout = Graphics::ChannelLayout::RG;
        rm.format.componentType = orm.format.componentType;
        rm.format.colorSpace = orm.format.colorSpace;
        rm.width = orm.width;
        rm.height = orm.height;
        rm.data.resize(rm.width * rm.height * rm.format.GetBytesPerPixel());

        const uint32_t componentSize = rm.format.GetComponentSize();
        const uint32_t ormBytesPerPixel = orm.format.GetBytesPerPixel();
        const uint32_t rmBytesPerPixel = rm.format.GetBytesPerPixel();

        const uint8_t* ormPtr = orm.data.data();
        uint8_t* rmPtr = rm.data.data();

        for (size_t offset = 0; offset < orm.data.size(); offset += ormBytesPerPixel)
        {
            std::memcpy(rmPtr, ormPtr + componentSize, componentSize * 2);

            ormPtr += ormBytesPerPixel;
            rmPtr += rmBytesPerPixel;
        }
        
        return rm;
    }
}