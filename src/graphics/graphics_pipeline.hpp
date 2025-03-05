#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "device.hpp"
#include "shader.hpp"
#include "containers.hpp"

namespace graphics
{
    struct PushConstants
    {
        alignas(64) glm::mat4 model;
        // alignas(4) float time;
    };
    
    class GraphicsPipeline
    {
    public:
        GraphicsPipeline(Device &_device, Shader& _shader);
        GraphicsPipeline(Device &_device, const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
        ~GraphicsPipeline();

        GraphicsPipeline(const GraphicsPipeline&) = delete;
        GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        VkPipelineLayout getPipelineLayout() { return pipelineLayout; }

    private:
        void createGraphicsPipeline();
        void createGraphicsPipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo);
        void createPipelineLayout();

        Device &device;
        VkPipeline m_graphicsPipeline;
        Shader &shader;
        VkPipelineCache pipelineCache;
        VkPipelineLayout pipelineLayout;
    };
}