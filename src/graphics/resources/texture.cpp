#include "texture.hpp"
#include "graphics/graphics_data.hpp"
#include "glm/gtc/integer.hpp"

namespace graphics
{

ImageProperties GetImageProperties(const TextureData &texture)
{
    const TextureConfig& config = texture.properties;
    ImageProperties props{};
    VkFormat fmt = ImageFormatToVkFormat(config.format);
    if(fmt != VK_FORMAT_UNDEFINED)
        props.format = vk::Format(fmt);
    else
    {
        Console::error("Unsupported format", "Texture");
        return props;
    }
    switch(config.type) // TODO: Make more of these options configurable
    {
    default:
    case TextureType::Default:
        props.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
        props.imageType = vk::ImageType::e2D;
        props.imageViewType = vk::ImageViewType::e2D;
        props.sampleCount = vk::SampleCountFlagBits::e1;
        props.arrayLayers = 1;
        break;
    case TextureType::NormalMap:
        props.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
        props.imageType = vk::ImageType::e2D;
        props.imageViewType = vk::ImageViewType::e2D;
        props.sampleCount = vk::SampleCountFlagBits::e1;
        props.arrayLayers = 1;
        break;
    case TextureType::CubeMap:
        props.usage = vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst;
        props.imageType = vk::ImageType::e2D;
        props.imageViewType = vk::ImageViewType::eCube;
        props.sampleCount = vk::SampleCountFlagBits::e1;
        props.arrayLayers = 1;
    }

    if(texture.properties.generateMipmaps)
    {
        props.mipLevels = glm::floor(glm::log2(glm::max(texture.GetWidth(), texture.GetHeight()))) + 1;
        props.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;
    }
    else
    {
        props.mipLevels = 1;
        props.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    }

    return props;
}

Texture::Texture(internal::Device &_device, const TextureData &texture)
    : device(_device),
    image(device, texture, GetImageProperties(texture)),
    sampler(device, texture.properties)
{
    Console::debugf("Created texture: {}", texture.name);
}

Texture::Texture(const TextureData &texture)
    : Texture(graphicsData->GetBackend().GetDevice(), texture)
{

}

} // namespace graphics