#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "graphics/internal/device.hpp"
#include "graphics/shader.hpp"
#include "graphics/containers.hpp"

namespace graphics
{
    struct PushConstants
    {
        // alignas(64) glm::mat4 model; // 64 bytes
        int objectID = -2; // 4 bytes
        // alignas(4) float time;
    };
    
    class GraphicsPipeline
    {
    public:
        GraphicsPipeline(Shader& _shader, int id, VkPipelineCache cache);
        // GraphicsPipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo, int id, VkPipelineLayout layout);
        ~GraphicsPipeline();

        GraphicsPipeline(const GraphicsPipeline&) = delete;
        GraphicsPipeline& operator=(const GraphicsPipeline&) = delete;

        void bind(VkCommandBuffer commandBuffer);

        int getID() const { return ID; }
        VkPipelineLayout getPipelineLayout() { return pipelineLayout; }

    protected:
        void createStandardPipeline(VkPipelineCache cache);
        void createPostProcessingPipeline(VkPipelineCache cache);
        void createIDBufferPipeline(VkPipelineCache cache);
        // void createGraphicsPipeline(const std::string& vertPath, const std::string& fragPath, const PipelineConfigInfo& configInfo, VkPipelineLayout layout);
        void createStandardLayout();
        void createPostProcessingLayout();
        void createIDBufferLayout();

        VkPipeline m_graphicsPipeline;
        VkPipelineLayout pipelineLayout;
        Shader &shader;

        int ID = -1;
    };
}