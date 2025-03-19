#include "compute_shader.hpp"
#include "../utils/file_util.hpp"
#include "containers.hpp"

#include <iostream>

namespace graphics
{
    ComputeShader::ComputeShader(const std::string &_path, std::vector<ShaderInput> _inputs) : 
        ShaderBase(_inputs), path(_path)
    {
        initializeDefaultConfigInfo();
        reloadShader();
    }

    ComputeShader::~ComputeShader()
    {
        vkDestroyShaderModule(Shared::device->device(), computeShaderModule, nullptr);
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
            vkDestroyShaderModule(Shared::device->device(), computeShaderModule, nullptr);
        }

        std::vector<char> code = file_util::readFileToCharVector(path);
        createShaderModule(code, &computeShaderModule);
    }
} // namespace graphics