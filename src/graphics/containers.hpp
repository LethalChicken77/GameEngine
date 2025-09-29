#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include "buffer.hpp"
#include "texture.hpp"
#include "descriptors.hpp"
#include "material.hpp"
#include "shader.hpp"
// #include "compute_shader.hpp"


namespace graphics
{
    #define GR_MAX_MATERIAL_COUNT 128
    // Forward declaration of Material class
    class Material;
    class Shader;
    class ComputeShader;
    namespace Shared
    {
        extern VkInstance instance;
        extern Device *device;
        extern std::vector<Material> materials;
        extern std::vector<std::unique_ptr<Shader>> shaders;
        // std::vector<ComputeShader> computeShaders;
    }
    // Declaration order matters
    // Global Descriptor Set
    namespace Descriptors
    {
        extern std::unique_ptr<DescriptorPool> globalPool;
        extern std::unique_ptr<DescriptorSetLayout> globalSetLayout;
        extern VkDescriptorSet globalDescriptorSet;
        // Camera Descriptor Set
        extern std::unique_ptr<DescriptorPool> cameraPool;
        extern std::unique_ptr<DescriptorSetLayout> cameraSetLayout;
        extern std::vector<VkDescriptorSet> cameraDescriptorSets;
        // ImGui Sets
        extern std::unique_ptr<DescriptorPool> imguiPool;
    }
} // namespace graphics