#include "shader_base.hpp"
#include "containers.hpp"

namespace graphics
{
    void ShaderBase::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
    {
        std::cout << "Creating shader module" << std::endl;
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        assert(code.size() % 4 == 0 && "Shader code size must be a multiple of 4");
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if(vkCreateShaderModule(Shared::device->device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create shader module!");
        }
    }
} // namespace graphics