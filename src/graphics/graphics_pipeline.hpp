#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "device.hpp"
#include "shader.hpp"

namespace graphics
{
    class GraphicsPipeline
    {
    public:
        GraphicsPipeline(Device &_device, Shader& _shader);
        GraphicsPipeline(Device &_device, const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
        ~GraphicsPipeline();

        GraphicsPipeline(const GraphicsPipeline&) = delete;
        GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

        void bind(VkCommandBuffer commandBuffer);

    private:
        void createGraphicsPipeline();
        void createGraphicsPipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
        void createPipelineLayout();

        Device &device;
        VkPipeline m_graphicsPipeline;
        Shader &shader;
        VkPipelineCache pipelineCache;
    };
}