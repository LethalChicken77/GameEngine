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
    ComputePipeline::ComputePipeline(ComputeShader &_shader) : shader(_shader)
    {
        createPipelineLayout();
        createComputePipeline();
    }

    ComputePipeline::~ComputePipeline()
    {
        vkDestroyPipeline(Shared::device->device(), m_computePipeline, nullptr);
        vkDestroyPipelineCache(Shared::device->device(), pipelineCache, nullptr);
        vkDestroyPipelineLayout(Shared::device->device(), pipelineLayout, nullptr);
    }

    void ComputePipeline::createPipelineLayout()
    {
        std::cout << "Creating compute pipeline layout" << std::endl;

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(ErosionPushConstants);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
            // Descriptors::globalSetLayout->getDescriptorSetLayout()
        };
        // for(auto &m : Shared::materials)
        // {
            descriptorSetLayouts.push_back(shader.getDescriptorSetLayout()->getDescriptorSetLayout());
        // }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if(vkCreatePipelineLayout(Shared::device->device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create compute pipeline layout");
        }
    }

    void ComputePipeline::createComputePipeline()
    {
        // TODO: Implement hot reloading of shaders
        ComputePipelineConfigInfo &configInfo = shader.getConfigInfo();
        
        assert(pipelineLayout != nullptr && "Cannot create compute pipeline:: layout is null");

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
        pipelineInfo.layout = pipelineLayout;
        
        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        VkPipelineCacheCreateInfo cacheCreateInfo{};
        cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

        if(vkCreatePipelineCache(Shared::device->device(), &cacheCreateInfo, nullptr, &pipelineCache) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create compute pipeline cache!");
        }
        cout << "Creating Compute Pipeline" << endl;
        if(vkCreateComputePipelines(Shared::device->device(), pipelineCache, 1, &pipelineInfo, nullptr, &m_computePipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create compute pipeline! (Skill issue)");
        }
        cout << "Created compute pipeline successfully" << endl;
    }

    void ComputePipeline::bind(VkCommandBuffer commandBuffer)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipeline);
    }

    void ComputePipeline::dispatch(VkCommandBuffer cmdBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ, bool wait)
    {
        // VkCommandBuffer cmdBuffer = Shared::device->beginSingleTimeCommands();
        
        // Bind the compute pipeline
        vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipeline);

        // Dispatch compute work with the specified number of workgroups
        vkCmdDispatch(cmdBuffer, groupCountX, groupCountY, groupCountZ);

        // if(!wait)
        // {
            // Shared::device->endSingleTimeCommands(cmdBuffer);
        //     return;
        // }
    }
} // namespace graphics