#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <memory>
#include "buffer.hpp"
#include "descriptors.hpp"
#include "utils.hpp"
#include "stb_image.h"
#include "device.hpp"

namespace graphics
{
    struct TextureProperties
    {
        VkFormat format; // Data format
        VkImageTiling tiling;
        VkImageUsageFlags usage;
        VkImageType imageType;
        VkImageViewType imageViewType;
        VkImageLayout finalLayout;
        VkSampleCountFlagBits sampleCount;
        VkMemoryPropertyFlags memoryProperties;

        TextureProperties getDefaultProperties()
        {
            return {
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                VK_IMAGE_TYPE_2D,
                VK_IMAGE_VIEW_TYPE_2D,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_SAMPLE_COUNT_1_BIT,
                VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
            };
        }
    };

    struct SamplerProperties
    {
        VkFilter magFilter; // Filter when supersampling
        VkFilter minFilter; // Filter when subsampling
        VkSamplerAddressMode addressMode;

        SamplerProperties getDefaultProperties()
        {
            return {
                VK_FILTER_LINEAR,
                VK_FILTER_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_REPEAT
            };
        }
    };
    
    class Texture
    {
    public:
        Texture();
        Texture(TextureProperties properties, uint32_t _width, uint32_t _height);
        Texture(const Texture &) = delete;
        Texture &operator=(const Texture &) = delete;
        Texture(Texture &&) = default;
        Texture &operator=(Texture &&) = default;
        ~Texture();

        static std::shared_ptr<Texture> loadFromFile(const std::string &filename); // Use stb_image to load image

        VkImageView getImageView() const { return imageView; }
        VkSampler getSampler() const { return sampler; }
        
        void createTexture();
        VkDescriptorImageInfo* getDescriptorInfo() { return &descriptorInfo; }

    private:
        VkCommandPool commandPool;
        VkQueue queue;
        
        VkImage image;
        VkDeviceMemory imageMemory;
        VkImageView imageView;
        VkSampler sampler;

        VkDescriptorImageInfo descriptorInfo;

        TextureProperties properties;
        SamplerProperties samplerProperties;

        std::vector<uint8_t> data{}; // Data stored in binary format, can be interpreted as whatever
        // std::unique_ptr<Buffer> buffer;

        uint32_t width;
        uint32_t height;
        uint32_t depth;

        void createImage();
        void allocateMemory();
        void createImageView();
        void createSampler();
        void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
        void copyDataToImage();
        void createDescriptorInfo();

        void cleanup();
    };
} // namespace graphics