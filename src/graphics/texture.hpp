#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <memory>
#include "containers.hpp"

namespace graphics
{
    struct TextureProperties
    {
        VkFormat format;
        VkImageTiling tiling;
        VkImageUsageFlags usage;
        VkMemoryPropertyFlags properties;

        TextureProperties getDefaultProperties()
        {
            return {
                VK_FORMAT_R8G8B8A8_SRGB,
                VK_IMAGE_TILING_OPTIMAL,
                VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
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
        Texture(TextureProperties properties, uint32_t _width, uint32_t _height);
        Texture(const Texture &) = delete;
        Texture &operator=(const Texture &) = delete;
        Texture(Texture &&) = default;
        Texture &operator=(Texture &&) = default;
        ~Texture() = default;

        static std::shared_ptr<Texture> loadFromFile(const std::string &filename); // Use stb_image to load image

        VkImageView getImageView() const { return imageView; }
        VkSampler getSampler() const { return sampler; }
    private:
        VkCommandPool commandPool;
        VkQueue queue;
        
        VkImage image;
        VkDeviceMemory imageMemory;
        VkImageView imageView;
        VkSampler sampler;

        uint32_t width;
        uint32_t height;

        void CreateImage(uint32_t width, uint32_t height, VkFormat format);
        void AllocateMemory();
        void CreateImageView();
        void CreateSampler();
        void UploadTextureData(unsigned char* pixels, uint32_t width, uint32_t height);
        void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
    };
} // namespace graphics