#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>

#include "graphics/containers.hpp"
#include "graphics/internal/device.hpp"
#include "graphics/internal/renderer.hpp"
#include "graphics/shader.hpp"
#include "graphics_pipeline.hpp"
#include "core/game_object.hpp"

namespace graphics{

class PipelineManager
{
    public:
        PipelineManager(Renderer& _renderer);
        ~PipelineManager();

        void createPipelines();
        void destroyPipelines();
        void reloadPipelines();

        std::unique_ptr<GraphicsPipeline> &getPipeline(uint32_t index) { return graphicsPipelines[index]; }

    private:
        Renderer& renderer;
        std::vector<std::unique_ptr<GraphicsPipeline>> graphicsPipelines;

        uint32_t currentID = 0;

        VkPipelineCache pipelineCache;

        void createPipelineCache();
};
} // namespace graphics