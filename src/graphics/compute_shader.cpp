#include "compute_shader.hpp"
#include "../utils/file_util.hpp"

#include <iostream>

namespace graphics
{
    ComputeShader::ComputeShader(Device &_device, const std::string &_path, std::vector<ShaderInput> inputs) : 
        device(_device), path(_path), inputs(inputs)
    {
        initializeDefaultConfigInfo();
        reloadShader();
    }

    ComputeShader::~ComputeShader()
    {
        vkDestroyShaderModule(device.device(), computeShaderModule, nullptr);
    }

    void ComputeShader::initializeDefaultConfigInfo()
    {
        // configInfo.pipelineLayout = nullptr;
    }
    
    void ComputeShader::reloadShader()
    {
        if(computeShaderModule != VK_NULL_HANDLE)
        {
            dirty = true;
            vkDestroyShaderModule(device.device(), computeShaderModule, nullptr);
        }

        std::vector<char> code = file_util::readFileToCharVector(path);
        createShaderModule(code, &computeShaderModule);
    }

    void ComputeShader::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
    {
        std::cout << "Creating shader module" << std::endl;
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        assert(code.size() % 4 == 0 && "Shader code size must be a multiple of 4");
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

        if(vkCreateShaderModule(device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create shader module!");
        }
    }
} // namespace graphics