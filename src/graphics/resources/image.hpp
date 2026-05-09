#pragma once
#include <unordered_map>

#include "graphics/backend/vulkan_include.h"

#include "graphics/api/resources/texture_data.hpp"
#include "graphics/backend/device.hpp"
#include "buffer.hpp"

namespace graphics
{

const VkFormat ImageFormatToVkFormat(ImageFormat fmt);
const ImageFormat VkFormatToImageFormat(VkFormat vkFmt);

struct ImageProperties
{
    vk::Format format = vk::Format::eR8G8B8A8Srgb; // Data format
    vk::ImageTiling tiling = vk::ImageTiling::eOptimal;
    vk::ImageUsageFlags usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eSampled;
    vk::ImageType imageType = vk::ImageType::e2D;
    vk::ImageViewType imageViewType = vk::ImageViewType::e2D;
    vk::SampleCountFlagBits sampleCount = vk::SampleCountFlagBits::e1;
    uint32_t mipLevels = 1;
    uint32_t arrayLayers = 1;
    vk::SharingMode sharingMode = vk::SharingMode::eExclusive;

    inline vk::ImageAspectFlags GetAspectMask() const noexcept
    {
        switch(format)
        {
            case vk::Format::eD32Sfloat:
            case vk::Format::eD16Unorm:
            case vk::Format::eX8D24UnormPack32:
                return vk::ImageAspectFlagBits::eDepth;
            
            case vk::Format::eD16UnormS8Uint:
            case vk::Format::eD24UnormS8Uint:
            case vk::Format::eD32SfloatS8Uint:
                return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
            
            case vk::Format::eS8Uint:
                return vk::ImageAspectFlagBits::eStencil;
            
            case vk::Format::eUndefined:
                // Console::warn("Cannot determine image aspect mask from undefined format.");
                return {};

            default:
                return vk::ImageAspectFlagBits::eColor;
        }
    }
};

class Image
{
public:
    Image(
        vk::DeviceSize width,
        vk::DeviceSize height,
        const ImageProperties &properties = {},
        vk::MemoryPropertyFlags memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal
    );
    Image(
        internal::Device &device,
        vk::DeviceSize width,
        vk::DeviceSize height,
        const ImageProperties &properties = {},
        vk::MemoryPropertyFlags memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal
    );
    Image(
        vk::DeviceSize width,
        vk::DeviceSize height,
        vk::DeviceSize depth,
        const ImageProperties &properties = {},
        vk::MemoryPropertyFlags memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal
    );
    Image(
        internal::Device &device,
        vk::DeviceSize width,
        vk::DeviceSize height,
        vk::DeviceSize depth,
        const ImageProperties &properties = {},
        vk::MemoryPropertyFlags memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    Image(
        const TextureData &textureData,
        const ImageProperties &properties = {},
        vk::MemoryPropertyFlags memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal
    );
    Image(
        internal::Device &device,
        const TextureData &textureData,
        const ImageProperties &properties = {},
        vk::MemoryPropertyFlags memoryProperties = vk::MemoryPropertyFlagBits::eDeviceLocal
    );

    ~Image();

    Image(const Image&);
    Image(const Image&, internal::Device& device);
    Image& operator=(const Image&) = delete;

    static void TransitionImageLayout(Image& image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::CommandBuffer commandBuffer);

    void TransitionImageLayout(vk::ImageLayout newLayout);
    void TransitionImageLayout(vk::ImageLayout newLayout, vk::CommandBuffer commandBuffer);
    void TransitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
    void TransitionImageLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::CommandBuffer commandBuffer);
    static void TransitionVkImageLayout(internal::Device& device, VkImage& image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange);
    static void TransitionVkImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer commandBuffer, VkImageSubresourceRange subresourceRange);
    static inline void TransitionVkImageLayout(vk::Image image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::CommandBuffer commandBuffer, vk::ImageSubresourceRange subresourceRange)
    {
        TransitionVkImageLayout(
            static_cast<VkImage>(image), 
            static_cast<VkImageLayout>(oldLayout),
            static_cast<VkImageLayout>(newLayout),
            static_cast<VkCommandBuffer>(commandBuffer),
            static_cast<VkImageSubresourceRange>(subresourceRange)
        );
    }
    /// @brief Reset to default image layout
    inline void ResetImageLayout() { TransitionImageLayout(defaultLayout); }
    /// @brief Reset to default image layout with command buffer
    inline void ResetImageLayout(vk::CommandBuffer commandBuffer) { TransitionImageLayout(defaultLayout, commandBuffer); }

    void CopyFromBuffer(const Buffer& buffer, uint32_t width, uint32_t height, uint32_t layerCount);

    void SetData(const TextureData &data);
    void GetData(TextureData *data) const;
    inline vk::Image GetImage() const noexcept { return image; }
    inline vk::ImageView GetImageView() const noexcept { return imageView; }
    inline vk::ImageSubresourceRange GetSubresourceRange() const noexcept
    {
        vk::ImageSubresourceRange range{};
        range.aspectMask = properties.GetAspectMask();
        range.baseMipLevel = 0;
        range.levelCount = properties.mipLevels;
        range.baseArrayLayer = 0;
        range.layerCount = 1;
        return range;
    }


    /// @brief Get Vulkan descriptor for this image. Does not include a sampler.
    /// @return Vulkan descriptor image info
    vk::DescriptorImageInfo GetDescriptorInfo() const
    {
        vk::DescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = currentLayout;
        imageInfo.imageView = imageView;
        imageInfo.sampler = VK_NULL_HANDLE;
        return imageInfo;
    }
private:
    internal::Device &device;
    ImageFormat format{};

    vk::Image image = VK_NULL_HANDLE;
    vk::ImageView imageView = VK_NULL_HANDLE;
    vma::Allocation imageAllocation = VK_NULL_HANDLE;
    vma::AllocationInfo imageAllocationInfo;
    
    ImageProperties properties{};
    vk::ImageLayout currentLayout = vk::ImageLayout::eUndefined;
    vk::ImageLayout defaultLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    vk::MemoryPropertyFlags memoryPropertyFlags{};

    uint32_t width = 0;
    uint32_t height = 0;
    uint32_t depth = 0;

    void create();
    void createInitialized(const TextureData& data);
    void createImage();
    void createImageView();
    void copyDataToImage();

    void generateMipmaps();

    friend class Buffer;
};
} // namespace graphics::internal