#include <string>
#include <iostream>
#include <vector>
#include <vulkan/vulkan.h>
#include <cassert>

#include "graphics_pipeline.hpp"
#include "../utils/file_util.hpp"
#include "mesh.hpp"

using namespace std;

namespace graphics
{    
    GraphicsPipeline::GraphicsPipeline(Shader &_shader, int id, VkPipelineCache cache) : shader(_shader), ID(id)
    {
        createPipelineLayout();
        createGraphicsPipeline(cache);
    }

    GraphicsPipeline::~GraphicsPipeline()
    {
        vkDestroyPipeline(Shared::device->device(), m_graphicsPipeline, nullptr);
        if(pipelineLayout != nullptr)
            vkDestroyPipelineLayout(Shared::device->device(), pipelineLayout, nullptr);
    }

    void GraphicsPipeline::createPipelineLayout()
    {
        std::cout << "Creating pipeline layout" << std::endl;

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstants);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
            Descriptors::globalSetLayout->getDescriptorSetLayout()
        };
        // for(std::unique_ptr<Shader>& shader : Shared::shaders)
        // {
            descriptorSetLayouts.push_back(shader.getDescriptorSetLayout()->getDescriptorSetLayout());
        // }
        // for(std::unique_ptr<Shader>& shader : Shared::shaders)
        // {
        //     descriptorSetLayouts.push_back(shader->getDescriptorSetLayout()->getDescriptorSetLayout());
        // }

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        if(vkCreatePipelineLayout(Shared::device->device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create pipeline layout");
        }
    }

    void GraphicsPipeline::createGraphicsPipeline(VkPipelineCache cache)
    {
        // TODO: Implement hot reloading of shaders
        shader.parentPipeline = this;
        PipelineConfigInfo &configInfo = shader.getConfigInfo();
        
        assert(pipelineLayout != nullptr && "Cannot create graphics pipeline:: layout is null");
        assert(configInfo.renderPass != nullptr && "Cannot create graphics pipeline:: render pass is null");

        VkPipelineShaderStageCreateInfo shaderStages[2];
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = shader.getVertexModule();
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;

        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = shader.getFragmentModule();
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;

        std::vector<VkVertexInputBindingDescription> bindingDescriptions = Mesh::Vertex::getBindingDescriptions();
        std::vector<VkVertexInputAttributeDescription> attributeDescriptions = Mesh::Vertex::getAttributeDescriptions();
        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
        pipelineInfo.pViewportState = &configInfo.viewportInfo;
        pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
        pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
        pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
        pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
        pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = configInfo.renderPass;
        pipelineInfo.subpass = configInfo.subpass;

        
        pipelineInfo.basePipelineIndex = -1;
        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

        cout << "Creating Graphics Pipeline" << endl;
        if(vkCreateGraphicsPipelines(Shared::device->device(), cache, 1, &pipelineInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create graphics pipeline! (Skill issue)");
        }
        cout << "Created graphics pipeline successfully" << endl;
    }

    void GraphicsPipeline::bind(VkCommandBuffer commandBuffer)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
    }
}