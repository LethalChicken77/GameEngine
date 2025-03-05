#include <string>
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include <cassert>

#include "compute_pipeline.hpp"
#include "../utils/file_util.hpp"
#include "mesh.hpp"

using namespace std;

namespace graphics
{
    ComputePipeline::ComputePipeline(Device &_device, ComputeShader &_shader) : device(_device), shader(_shader)
    {
        createComputePipeline();
    }

    ComputePipeline::~ComputePipeline()
    {
        vkDestroyPipeline(device.device(), m_computePipeline, nullptr);
        vkDestroyPipelineCache(device.device(), pipelineCache, nullptr);
    }

    void ComputePipeline::createComputePipeline()
    {
        // TODO: Implement hot reloading of shaders
        ComputePipelineConfigInfo &configInfo = shader.getConfigInfo();
        
        assert(configInfo.pipelineLayout != nullptr && "Cannot create compute pipeline:: layout is null");

        VkPipelineShaderStageCreateInfo shaderStages[1]{};
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_COMPUTE_BIT;
        shaderStages[0].module = shader.getShaderModule();
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;
        
        VkComputePipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        pipelineInfo.stage = shaderStages[0];
        pipelineInfo.layout = configInfo.pipelineLayout;
        
        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        VkPipelineCacheCreateInfo cacheCreateInfo{};
        cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

        if(vkCreatePipelineCache(device.device(), &cacheCreateInfo, nullptr, &pipelineCache) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create compute pipeline cache!");
        }
        cout << "Creating Compute Pipeline" << endl;
        if(vkCreateComputePipelines(device.device(), pipelineCache, 1, &pipelineInfo, nullptr, &m_computePipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create compute pipeline! (Skill issue)");
        }
        cout << "Created compute pipeline successfully" << endl;
    }

    void ComputePipeline::bind(VkCommandBuffer commandBuffer)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipeline);
    }
} // namespace graphics