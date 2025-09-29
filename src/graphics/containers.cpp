#include "containers.hpp"

namespace graphics
{
namespace Shared
{
    VkInstance instance;
    Device *device;
    
    std::vector<Material> materials{};
    std::vector<std::unique_ptr<Shader>> shaders{};
}

namespace Descriptors
{
// Declaration order matters
// Global Descriptor Set
std::unique_ptr<DescriptorPool> globalPool;
std::unique_ptr<DescriptorSetLayout> globalSetLayout;
VkDescriptorSet globalDescriptorSet;
// Camera Descriptor Set
std::unique_ptr<DescriptorPool> cameraPool;
std::unique_ptr<DescriptorSetLayout> cameraSetLayout;
std::vector<VkDescriptorSet> cameraDescriptorSets;
// ImGui Sets
std::unique_ptr<DescriptorPool> imguiPool;
}
} // namespace graphics