#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "frame_info.hpp"
#include "containers.hpp"
#include "device.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include "graphics_pipeline.hpp"
#include "../core/game_object.hpp"

namespace graphics{

class PipelineManager
{
    public:
        PipelineManager(Renderer& _renderer);
        ~PipelineManager();

        void createPipelines();
        void destroyPipelines();
        void reloadPipelines();

        void renderObjects(FrameInfo& frameInfo, std::vector<core::GameObject_t>& gameObjects, VkCommandBuffer& commandBuffer);

        std::unique_ptr<GraphicsPipeline> &getPipeline(uint32_t index) { return graphicsPipelines[index]; }

    private:
        Renderer& renderer;
        std::vector<std::unique_ptr<GraphicsPipeline>> graphicsPipelines;

        uint32_t currentID = 0;

        VkPipelineCache pipelineCache;

        void createPipelineCache();
};
} // namespace graphics