#include "compute_shader.hpp"
#include "../utils/file_util.hpp"
#include "containers.hpp"

#include <iostream>

namespace graphics
{
    ComputeShader::ComputeShader(const std::string &_path, std::vector<ShaderInput> _inputs, uint32_t textureCount) : 
        ShaderBase(_inputs), path(_path)
    {
        initializeDefaultConfigInfo();
        reloadShader();


        DescriptorPool::Builder poolBuilder = DescriptorPool::Builder(*Shared::device)
            .setMaxSets(GR_MAX_MATERIAL_COUNT)
            .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, GR_MAX_MATERIAL_COUNT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, GR_MAX_MATERIAL_COUNT);
        DescriptorSetLayout::Builder layoutBuilder = DescriptorSetLayout::Builder(*Shared::device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, VK_SHADER_STAGE_COMPUTE_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT);
        
        // for(int i = 0; i < textureCount; i++)
        // {
        //     poolBuilder.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, GR_MAX_MATERIAL_COUNT);
        //     layoutBuilder.addBinding(i + 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT);
        // }
        descriptorPool = poolBuilder.build();
        descriptorSetLayout = layoutBuilder.build();
    }

    ComputeShader::~ComputeShader()
    {
        vkDestroyShaderModule(Shared::device->device(), computeShaderModule, nullptr);
    }

    void ComputeShader::initializeDefaultConfigInfo()
    {
        
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