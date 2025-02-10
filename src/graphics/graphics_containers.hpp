#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "buffer.hpp"
#include "descriptors.hpp"

namespace graphics
{
    struct Containers
    {
        Device *device;
        // Declaration order matters
        // Global Descriptor Set
        std::unique_ptr<DescriptorPool> globalPool;
        std::unique_ptr<DescriptorSetLayout> globalSetLayout;
        std::vector<VkDescriptorSet> globalDescriptorSets;
        // Material Descriptor Set
        std::unique_ptr<DescriptorPool> materialPool;
        std::unique_ptr<DescriptorSetLayout> materialSetLayout;
        std::vector<VkDescriptorSet> materialDescriptorSets;
        // ImGui Sets
        std::unique_ptr<DescriptorPool> imguiPool;
    };
} // namespace graphics