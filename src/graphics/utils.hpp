#pragma once

#include <vulkan/vulkan.h>

namespace graphics
{
    namespace Utils
    {
        VkCommandBuffer beginSingleTimeCommands();
        void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    } // namespace Utils
} // namespace graphics