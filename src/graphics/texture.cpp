#include "texture.hpp"
#include "stb_image.h"
#include <iostream>

namespace graphics
{
    std::shared_ptr<Texture> Texture::loadFromFile(const std::string& path)
    {
        int texWidth, texHeight, texChannels;
        stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
        if(!pixels)
        {
            throw std::runtime_error("Failed to load texture image");
        }
        VkDeviceSize imageSize = texWidth * texHeight * 4;

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        // Buffer::CreateBuffer(
        //     imageSize,
        //     VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        //     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        //     stagingBuffer,
        //     stagingBufferMemory
        // );
    }
} // namespace graphics