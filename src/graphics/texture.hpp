#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <memory>
#include "buffer.hpp"
#include "descriptors.hpp"
#include "utils.hpp"
#include "stb_image.h"
#include "device.hpp"
#include "glm/glm.hpp"

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
        Texture(uint32_t _width, uint32_t _height);
        Texture(TextureProperties _properties, SamplerProperties _samplerProperties, uint32_t _width, uint32_t _height);
        Texture(const Texture &) = delete;
        Texture &operator=(const Texture &) = delete;
        Texture(Texture &&) = default;
        Texture &operator=(Texture &&) = default;
        ~Texture();

        static std::shared_ptr<Texture> loadFromFile(const std::string &filename); // Use stb_image to load image

        VkImageView getImageView() const { return imageView; }
        VkSampler getSampler() const { return sampler; }
        
        void setPixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
        void setPixel(uint32_t x, uint32_t y, float value);
        float getPixelFloat(uint32_t x, uint32_t y) const
        {
            if(properties.format != VK_FORMAT_R32_SFLOAT)
            {
                throw std::runtime_error("Cannot get pixel on non-float texture");
            }
            x = glm::clamp(x, 0u, width - 1);
            y = glm::clamp(y, 0u, height - 1);

            int dataIndex = (y * width + x) * sizeof(float);
            float result = 0.0f;
            memcpy(&result, &data[dataIndex], sizeof(float));
            return result;
        }
        void createTexture();
        void updateOnGPU();
        VkDescriptorImageInfo* getDescriptorInfo() { return &descriptorInfo; }

        // Need this public to swap between compute and graphics pipelines
        void transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);

        uint32_t getWidth() const { return width; }
        uint32_t getHeight() const { return height; }

        float sampleBilinear(float x, float y);
        float sampleBilinear(glm::vec3 pos)
        {
            return sampleBilinear(pos.x, pos.y);
        }

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
        void copyDataToImage();
        void createDescriptorInfo();

        void cleanup();
    };
} // namespace graphics