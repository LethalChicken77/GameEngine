#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "buffer.hpp"
#include "descriptors.hpp"
#include "material.hpp"


namespace graphics
{
    // Forward declaration of Material class
    class Material;
    namespace Shared
    {
        extern VkInstance instance;
        extern Device *device;
        extern std::vector<Material> materials;
    }
    // Declaration order matters
    // Global Descriptor Set
    namespace Descriptors
    {
        extern std::unique_ptr<DescriptorPool> globalPool;
        extern std::unique_ptr<DescriptorSetLayout> globalSetLayout;
        extern std::vector<VkDescriptorSet> globalDescriptorSets;
        // Material Descriptor Set
        extern std::unique_ptr<DescriptorPool> materialPool;
        extern std::unique_ptr<DescriptorSetLayout> materialSetLayout;
        extern std::vector<VkDescriptorSet> materialDescriptorSets;
        // ImGui Sets
        extern std::unique_ptr<DescriptorPool> imguiPool;
    }
} // namespace graphics