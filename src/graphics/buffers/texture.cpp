#include <iostream>
#include <unistd.h>
// #include <filesystem>
#include "texture.hpp"
#include "graphics/containers.hpp"
#define TINYEXR_IMPLEMENTATION
#include "tinyexr.h"
#include <format>

namespace graphics
{
    Texture::Texture(uint32_t _width, uint32_t _height)
    {
        // if(data.size() == 0)
        // {
        //     throw std::runtime_error("Cannot initialize empty texture");
        // }
        properties = TextureProperties::getDefaultProperties();
        samplerProperties = SamplerProperties::getDefaultProperties();
        width = _width;
        height = _height;
        data.resize(width * height * 4);
    }

    Texture::Texture(TextureProperties _properties, uint32_t _width, uint32_t _height)
    {
        properties = _properties;
        samplerProperties = SamplerProperties::getDefaultProperties();
        width = _width;
        height = _height;
        data.resize(width * height * 4);
    }
    
    Texture::Texture(TextureProperties _properties, SamplerProperties _samplerProperties, uint32_t _width, uint32_t _height)
    {
        properties = _properties;
        samplerProperties = _samplerProperties;
        width = _width;
        height = _height;
        data.resize(width * height * 4);
    }

    Texture::~Texture()
    {
        cleanup();
    }

    std::shared_ptr<Texture> Texture::loadFromFile(const std::string& path, VkFormat format)
    {
        // if(!std::filesystem::exists(path))
        if(access(path.c_str(), F_OK) != 0)
        {
            throw std::runtime_error("File not found: " + path);
        }
        
        stbi_set_flip_vertically_on_load(true); // Flip texture on load to match UV coords

        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if(!pixels)
        {
            throw std::runtime_error("Failed to load texture image");
        }
        uint32_t pixelCount = texWidth * texHeight;
        
        // int numChannels = texChannels; // Actual image channels (from STB)
        int numChannels = sizeof(STBI_rgb_alpha); // Force 4 Channels
        std::shared_ptr<Texture> texture = std::make_shared<Texture>(texWidth, texHeight);
        memcpy(texture->data.data(), pixels, pixelCount * numChannels * sizeof(stbi_uc));

        texture->width = texWidth;
        texture->height = texHeight;
        texture->properties = TextureProperties().getDefaultProperties();
        texture->samplerProperties = SamplerProperties().getDefaultProperties();
        texture->properties.format = format;

        texture->createTexture();

        return texture;
    }

    std::shared_ptr<Texture> Texture::loadFromFileEXR(const std::string& path, TextureProperties properties, SamplerProperties samplerProperties)
    {
        if (access(path.c_str(), F_OK) != 0)
        {
            throw std::runtime_error("[EXR Loader] File not found: " + path);
        }

        const char* err = nullptr;

        // Parse EXR version
        EXRVersion version;
        if (ParseEXRVersionFromFile(&version, path.c_str()) != TINYEXR_SUCCESS) {
            throw std::runtime_error("Failed to parse EXR version");
        }

        // Parse the EXR header
        EXRHeader header;
        InitEXRHeader(&header);
        if (ParseEXRHeaderFromFile(&header, &version, path.c_str(), &err) != TINYEXR_SUCCESS) {
            std::string errorMsg = err ? std::string(err) : "Unknown EXR header error";
            FreeEXRErrorMessage(err);
            throw std::runtime_error("Failed to parse EXR header: " + errorMsg);
        }

        // Request conversion to FLOAT for any HALF channels
        for (int i = 0; i < header.num_channels; ++i) {
            if (header.pixel_types[i] == TINYEXR_PIXELTYPE_HALF) {
                header.requested_pixel_types[i] = TINYEXR_PIXELTYPE_FLOAT;
            }
        }
        // Load the EXR image
        EXRImage image;
        if (LoadEXRImageFromFile(&image, &header, path.c_str(), &err) != TINYEXR_SUCCESS) {
            std::string errorMsg = err ? std::string(err) : "Unknown EXR image error";
            FreeEXRErrorMessage(err);
            throw std::runtime_error("Failed to load EXR image: " + errorMsg);
        }
        // Access image data
        int numChannels = image.num_channels;
        int texWidth = image.width;
        int texHeight = image.height;

        size_t rIndex, gIndex, bIndex, aIndex = -1;

        for(int i = 0; i < numChannels; i++)
        {
            if(strcmp(header.channels[i].name, "R") == 0 || strcmp(header.channels[i].name, "Y") == 0) rIndex = i;
            else if(strcmp(header.channels[i].name, "G") == 0) gIndex = i;
            else if(strcmp(header.channels[i].name, "B") == 0) bIndex = i;
            else if(strcmp(header.channels[i].name, "A") == 0) aIndex = i;
        }

        const uint32_t pixelCount = texWidth * texHeight;

        // Map channels by name
        std::vector<float> chR(pixelCount);
        memcpy(chR.data(), image.images[rIndex], sizeof(float) * pixelCount);
        std::vector<float> chG, chB, chA;
        if (numChannels >= 2) {
            chG.resize(pixelCount);
            memcpy(chG.data(), image.images[gIndex], sizeof(float) * pixelCount);
        }
        if (numChannels >= 3) {
            chB.resize(pixelCount);
            memcpy(chB.data(), image.images[bIndex], sizeof(float) * pixelCount);
        }
        if (numChannels >= 4) {
            chA.resize(pixelCount);
            memcpy(chA.data(), image.images[aIndex], sizeof(float) * pixelCount);
        }

        
        int effectiveNumChannels = (numChannels == 3) ? 4 : numChannels; // Expand RGB to RGBA
        std::shared_ptr<Texture> texture = std::make_shared<Texture>(texWidth, texHeight);

        texture->data.resize(pixelCount * sizeof(float) * effectiveNumChannels);

        for (uint32_t i = 0; i < pixelCount; i++)
        {
            // std::cout << "Pixel: " << i << " / " << pixelCount << "\n";
            float pixelData[4] = {
                chR[i],
                (numChannels >= 2) ? chG[i] : 0.0f,
                (numChannels >= 3) ? chB[i] : 0.0f,
                (numChannels >= 4) ? chA[i] : 1.0f
            };
            memcpy(
                texture->data.data() + i * sizeof(float) * effectiveNumChannels, 
                pixelData, 
                sizeof(float) * effectiveNumChannels
            );
        }
        
        if(!header.tiled)
        {
            image.num_tiles = 0; // Prevent tinyexr from trying to free tiles
            image.tiles = nullptr;
        }
        if(image.next_level)
        {
            image.next_level = nullptr; // Prevent tinyexr from trying to free mipmaps
        }
        // Free the image data when done
        FreeEXRImage(&image);
        FreeEXRHeader(&header);

        texture->width = texWidth;
        texture->height = texHeight;
        texture->properties = properties;
        texture->samplerProperties = samplerProperties;
        
        switch(numChannels)
        {
            case 1:
                texture->properties.format = VK_FORMAT_R32_SFLOAT;
                break;
            case 2:
                texture->properties.format = VK_FORMAT_R32G32_SFLOAT;
                break;
            case 3:
                texture->properties.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
            case 4:
                texture->properties.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
            default:
                texture->properties.format = VK_FORMAT_R32G32B32A32_SFLOAT;
                break;
        }

        texture->createTexture();

        return texture;
    }


    void Texture::saveToFileEXR(const std::string& path)
    {
        int pixelCount = width * height;
        int floatSize = sizeof(float);
        int expectedSize = pixelCount * floatSize;

        if (data.size() != expectedSize)
        {
            throw std::runtime_error("Data size doesn't match expected float pixel layout for EXR output.");
        }

        // Reinterpret the byte vector as float data
        const float* floatPixels = reinterpret_cast<const float*>(data.data());

        float* images[1];
        images[0] = const_cast<float*>(floatPixels); // TinyEXR is not const-correct

        EXRImage image;
        InitEXRImage(&image);
        image.num_channels = 1;
        image.images = reinterpret_cast<unsigned char**>(images);
        image.width = width;
        image.height = height;

        EXRHeader header;
        InitEXRHeader(&header);
        header.num_channels = 1;

        header.channels = (EXRChannelInfo*)malloc(sizeof(EXRChannelInfo));
        strcpy(header.channels[0].name, "Y"); // Luminance (grayscale)

        header.pixel_types = (int*)malloc(sizeof(int));
        header.requested_pixel_types = (int*)malloc(sizeof(int));
        header.pixel_types[0] = TINYEXR_PIXELTYPE_FLOAT;
        header.requested_pixel_types[0] = TINYEXR_PIXELTYPE_FLOAT;

        const char* err = nullptr;
        int result = SaveEXRImageToFile(&image, &header, path.c_str(), &err);

        // Cleanup regardless of result
        free(header.channels);
        free(header.pixel_types);
        free(header.requested_pixel_types);

        if (result != TINYEXR_SUCCESS)
        {
            std::string msg = "Failed to save EXR: ";
            if (err)
            {
                msg += err;
                FreeEXRErrorMessage(err);
            }
            throw std::runtime_error(msg);
        }
    }


    void Texture::setPixel(uint32_t x, uint32_t y, uint8_t r, uint8_t g, uint8_t b, uint8_t a)
    {
        if(properties.format != VK_FORMAT_R8G8B8A8_SRGB)
        {
            throw std::runtime_error("Cannot set pixel on non-RGBA texture");
        }
        int dataIndex = (y * width + x) * 4;
        data[dataIndex] = r;
        data[dataIndex + 1] = g;
        data[dataIndex + 2] = b;
        data[dataIndex + 3] = a;
        // return true;
    }

    void Texture::setPixel(uint32_t x, uint32_t y, float value)
    {
        if(properties.format != VK_FORMAT_R32_SFLOAT)
        {
            throw std::runtime_error("Cannot set float pixel on non-float texture");
        }
        x = glm::clamp(x, 0u, width - 1);
        y = glm::clamp(y, 0u, height - 1);
        // if(x >= width || y >= height)
        // {
        //     // std::cout << data.size() << std::endl;
        //     // std::cout << width << ", " << height << std::endl;
        //     // std::cerr << ("Pixel coordinates out of bounds " + std::to_string(x) + ", " + std::to_string(y)) << std::endl;
        // }
        int dataIndex = (y * width + x) * sizeof(float);
        memcpy(&data[dataIndex], &value, sizeof(float));
        // return true;
    }

    float Texture::sampleBilinear(float x, float y)
    {
        x = glm::clamp(x, 0.0f, 1.0f);
        y = glm::clamp(y, 0.0f, 1.0f);
        float u = x * width;
        float v = y * height;
        int x0 = (int)u;
        int y0 = (int)v;
        int x1 = (x0 + 1) % width; // Wrap around
        int y1 = (y0 + 1) % height;
        float s = u - x0;
        float t = v - y0;
        float c00;
        memcpy(&c00, &data[(y0 * width + x0) * 4], sizeof(float));
        float c01;
        memcpy(&c01, &data[(y1 * width + x0) * 4], sizeof(float));
        float c10;
        memcpy(&c10, &data[(y0 * width + x1) * 4], sizeof(float));
        float c11;
        memcpy(&c11, &data[(y1 * width + x1) * 4], sizeof(float));
        // Perform bilinear interpolation
        float c0 = c00 * (1 - s) + c01 * s;
        float c1 = c10 * (1 - s) + c11 * s;
        return c0 * (1 - t) + c1 * t;
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

    void Texture::createTextureUninitialized()
    {
        createImage();
        allocateMemory();
        transitionImageLayout(VK_IMAGE_LAYOUT_UNDEFINED, properties.finalLayout);
        createSampler();
        createImageView();

        createDescriptorInfo();
    }

    void Texture::updateOnGPU()
    {
        VkImageLayout prevLayout = currentLayout;
        transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        copyDataToImage();
        transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, prevLayout);
    }

    void Texture::updateOnCPU()
    {
        VkImageLayout prevLayout = currentLayout;
        transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        copyDataFromImage();
        transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, prevLayout);
    }
    
    void Texture::updatePixelOnCPU(uint32_t x, uint32_t y)
    {
        x = glm::clamp(x, 0u, width - 1);
        y = glm::clamp(y, 0u, height - 1);
        VkImageLayout prevLayout = currentLayout;
        transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
        copyPixelFromImage(x, y);
        transitionImageLayout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, prevLayout);
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
        imageInfo.sharingMode = properties.sharingMode;

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
        samplerInfo.borderColor = samplerProperties.borderColor;
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
        // viewInfo.subresourceRange = properties.imageSubResourceRange;
        viewInfo.subresourceRange.aspectMask = properties.imageSubResourceRange.aspectMask;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if(vkCreateImageView(Shared::device->device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create texture image view");
        }
    }

    void Texture::transitionImageLayout(VkImageLayout newLayout)
    {
        transitionImageLayout(currentLayout, newLayout);
    }

    void Texture::transitionImageLayout(VkImageLayout newLayout, VkCommandBuffer commmandBuffer)
    {
        transitionImageLayout(currentLayout, newLayout, commmandBuffer);
    }

    void Texture::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = Shared::device->beginSingleTimeCommands(); // Begin recording a command buffer
        transitionImageLayout(oldLayout, newLayout, commandBuffer);
        Shared::device->endSingleTimeCommands(commandBuffer); // Submit and free the command buffer
    }

    void Texture::transitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer commandBuffer)
    {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = properties.imageSubResourceRange.aspectMask;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
    
        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;
    
        // Handle oldLayout
        switch (oldLayout)
        {
            case VK_IMAGE_LAYOUT_UNDEFINED:
                barrier.srcAccessMask = 0;
                sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                break;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                break;
            case VK_IMAGE_LAYOUT_GENERAL:
                barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
                sourceStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
                break;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                sourceStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                sourceStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                sourceStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                barrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
                sourceStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                break;
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                barrier.srcAccessMask = 0; // Present doesn’t guarantee writes; usually treated as TOP_OF_PIPE
                sourceStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
                break;
            default:
                throw std::invalid_argument("Unsupported old layout transition: " + std::to_string((uint32_t)oldLayout));
        }

        // Handle newLayout
        switch (newLayout)
        {
            case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                break;
            case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
                destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
                break;
            case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                break;
            case VK_IMAGE_LAYOUT_GENERAL:
                barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
                destinationStage = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
                break;
            case VK_IMAGE_LAYOUT_UNDEFINED:
                barrier.dstAccessMask = 0;
                destinationStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
                break;
            case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
                destinationStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
                destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
                break;
            case VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL:
                barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
                destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                break;
            case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
                barrier.dstAccessMask = 0; // Present doesn’t require memory access mask
                destinationStage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
                break;
            default:
                throw std::invalid_argument("Unsupported new layout transition: " + std::to_string((uint32_t)newLayout));
        }
        
        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage, destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        currentLayout = newLayout;
        descriptorInfo.imageLayout = currentLayout;
    }

    size_t getFormatSize(VkFormat format)
    {
        switch (format)
        {
            case VK_FORMAT_R8_SRGB:
            case VK_FORMAT_R8_UNORM: return 1;

            case VK_FORMAT_R8G8_SRGB:
            case VK_FORMAT_R8G8_UNORM: return 2;

            case VK_FORMAT_R8G8B8_SRGB:
            case VK_FORMAT_R8G8B8_UNORM: return 3;

            case VK_FORMAT_R8G8B8A8_SRGB:
            case VK_FORMAT_R8G8B8A8_UNORM: return 4;

            case VK_FORMAT_R32_SFLOAT: return 4;
            case VK_FORMAT_R32G32_SFLOAT: return 8;
            case VK_FORMAT_R32G32B32_SFLOAT: return 12;
            case VK_FORMAT_R32G32B32A32_SFLOAT: return 16;

            // Add more as needed
            default: 
                throw std::runtime_error("Unknown or unsupported VkFormat: " + std::to_string(format));
        }
    }

    void Texture::copyDataToImage() 
    {
        uint32_t pixelCount = width * height;
        uint32_t pixelSize = sizeof(data[0]) * getFormatSize(properties.format);
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

    void Texture::copyDataFromImage()
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
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };
        
        stagingBuffer.map();
        // std::cout << (void *)data.data() << std::endl;
        
        // std::cout << "Copying to image" << std::endl;
        Shared::device->copyImageToBuffer(image, stagingBuffer.getBuffer(), width, height, 1);
        
        stagingBuffer.readFromBuffer((void *)data.data());
    }

    void Texture::copyPixelFromImage(uint32_t x, uint32_t y)
    {
        if(x >= width || y >= height || x < 0 || y < 0)
        {
            Console::error("Cannot copy pixel outside texture bounds: (" + std::to_string(x) + ", " + std::to_string(y) + ")", "Texture");
        }

        uint32_t pixelSize = sizeof(data[0]) * 4;
        VkDeviceSize bufferSize = pixelSize;

        // Ensure your data buffer has at least one pixel
        if(data.size() < bufferSize)
            throw std::runtime_error("Texture data buffer too small for a single pixel");

        // Create a staging buffer for a single pixel
        Buffer stagingBuffer{
            *Shared::device,
            pixelSize,
            1, // only one pixel
            VK_BUFFER_USAGE_TRANSFER_DST_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        };

        stagingBuffer.map();

        // Copy only the 1x1 region at (x,y)
        VkBufferImageCopy copyRegion{};
        copyRegion.bufferOffset = 0;
        copyRegion.bufferRowLength = 0;   // tightly packed
        copyRegion.bufferImageHeight = 0;
        copyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        copyRegion.imageSubresource.mipLevel = 0;
        copyRegion.imageSubresource.baseArrayLayer = 0;
        copyRegion.imageSubresource.layerCount = 1;
        copyRegion.imageOffset = { static_cast<int32_t>(x), static_cast<int32_t>(y), 0 };
        copyRegion.imageExtent = { 1, 1, 1 };

        Shared::device->copyImageToBuffer(image, stagingBuffer.getBuffer(), 1, 1, 1, copyRegion);

        // Read the pixel into your data buffer
        uint32_t pixelIndex = y * width + x;
        stagingBuffer.readFromBuffer((void*)(data.data() + pixelIndex * 4));
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