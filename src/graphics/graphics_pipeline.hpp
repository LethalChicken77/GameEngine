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
        GraphicsPipeline(Shader& _shader, int id, VkPipelineCache cache);
        GraphicsPipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo, int id, VkPipelineLayout layout);
        ~GraphicsPipeline();

        GraphicsPipeline(const GraphicsPipeline&) = delete;
        GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

        void bind(VkCommandBuffer commandBuffer);

        int getID() const { return ID; }
        VkPipelineLayout getPipelineLayout() { return pipelineLayout; }

    protected:
        void createGraphicsPipeline(VkPipelineCache cache);
        void createGraphicsPipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo, VkPipelineLayout layout);
        void createPipelineLayout();

        VkPipeline m_graphicsPipeline;
        VkPipelineLayout pipelineLayout;
        Shader &shader;

        int ID = -1;
    };
}