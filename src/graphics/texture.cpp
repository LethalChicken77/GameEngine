#include <iostream>
// #include <filesystem>
#include "texture.hpp"
#include "containers.hpp"
#include <unistd.h>

namespace graphics
{
    Texture::Texture()
    {
        // if(data.size() == 0)
        // {
        //     throw std::runtime_error("Cannot initialize empty texture");
        // }
    }

    Texture::~Texture()
    {
        cleanup();
    }

    std::shared_ptr<Texture> Texture::loadFromFile(const std::string& path)
    {
        // if(!std::filesystem::exists(path))
        if(access(path.c_str(), F_OK) != 0)
        {
            throw std::runtime_error("File not found: " + path);
        }
        std::shared_ptr<Texture> texture = std::make_shared<Texture>();
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if(!pixels)
        {
            throw std::runtime_error("Failed to load texture image");
        }
        uint32_t pixelCount = texWidth * texHeight;

        // int numChannels = texChannels; // Actual image channels (from STB)
        int numChannels = sizeof(STBI_rgb_alpha); // Force 4 Channels
        texture->data.resize(pixelCount * numChannels);
        memcpy(texture->data.data(), pixels, pixelCount * numChannels * sizeof(stbi_uc));

        texture->width = texWidth;
        texture->height = texHeight;
        texture->properties = TextureProperties().getDefaultProperties();
        texture->samplerProperties = SamplerProperties().getDefaultProperties();
        texture->properties.format = VK_FORMAT_R8G8B8A8_SRGB;

        texture->createTexture();

        return texture;
    }
    
    void Texture::createTexture()
    {
        createImage();
        allocateMemory();
        transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyDataToImage();
        transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, properties.finalLayout);
        createSampler();
        createImageView();

        createDescriptorInfo();
    }

    void Texture::allocateMemory()
    {
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(Shared::device->device(), image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = Shared::device->findMemoryType(memRequirements.memoryTypeBits, properties.memoryProperties);

        if(vkAllocateMemory(Shared::device->device(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate image memory");
        }

        if(vkBindImageMemory(Shared::device->device(), image, imageMemory, 0) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to bind image memory");
        }
    }

    void Texture::createImage()
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = properties.imageType;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = properties.format;
        imageInfo.tiling = properties.tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = properties.usage;
        imageInfo.samples = properties.sampleCount;

        if(vkCreateImage(Shared::device->device(), &imageInfo, nullptr, &image) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image");
        }
    }

    void Texture::createSampler()
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = samplerProperties.magFilter;
        samplerInfo.minFilter = samplerProperties.minFilter;
        samplerInfo.addressModeU = samplerProperties.addressMode;
        samplerInfo.addressModeV = samplerProperties.addressMode;
        samplerInfo.addressModeW = samplerProperties.addressMode;
        samplerInfo.anisotropyEnable = VK_TRUE; // Add fields to properties
        samplerInfo.maxAnisotropy = 16;
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;

        if(vkCreateSampler(Shared::device->device(), &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create texture sampler");
        }
    }
    
    void Texture::createImageView()
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = properties.imageViewType;
        viewInfo.format = properties.format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if(vkCreateImageView(Shared::device->device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create texture image view");
        }
    }

    void Texture::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = Shared::device->beginSingleTimeCommands(); // Begin recording a command buffer

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
    
        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;
    
        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        } else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        } else {
            throw std::invalid_argument("Unsupported layout transition!");
        }
    
        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
    
        Shared::device->endSingleTimeCommands(commandBuffer); // Submit and free the command buffer
    }

    void Texture::copyDataToImage() 
    {
        uint32_t pixelCount = width * height;
        uint32_t pixelSize = sizeof(data[0]) * 4;
        VkDeviceSize bufferSize = pixelSize * pixelCount;
        // std::cout << "Buffer size: " << bufferSize << std::endl;
        // std::cout << "Data size: " << data.size() << std::endl;
        // std::cout << "Width: " << width << " Height: " << height << std::endl;
        if(data.size() != bufferSize)
        {
            throw std::runtime_error("Texture data size does not match buffer size");
        }
        
        Buffer stagingBuffer{
            *Shared::device,
            pixelSize,
            pixelCount,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };
        
        stagingBuffer.map();
        // std::cout << (void *)data.data() << std::endl;
        stagingBuffer.writeToBuffer((void *)data.data());
        
        // std::cout << "Copying to image" << std::endl;
        Shared::device->copyBufferToImage(stagingBuffer.getBuffer(), image, width, height, 1);
    }

    void Texture::createDescriptorInfo()
    {
        descriptorInfo.imageLayout = properties.finalLayout;
        descriptorInfo.imageView = imageView;
        descriptorInfo.sampler = sampler;
    }

    void Texture::cleanup()
    {
        vkDestroySampler(Shared::device->device(), sampler, nullptr);
        vkDestroyImageView(Shared::device->device(), imageView, nullptr);
        vkDestroyImage(Shared::device->device(), image, nullptr);
        vkFreeMemory(Shared::device->device(), imageMemory, nullptr);
    }
} // namespace graphics