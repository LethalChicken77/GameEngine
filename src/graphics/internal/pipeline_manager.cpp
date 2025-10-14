#include "pipeline_manager.hpp"
#include "utils/console.hpp"

namespace graphics{

PipelineManager::PipelineManager(Renderer& _renderer) : renderer(_renderer)
{
    // createPipelineLayout();
    createPipelineCache();
    createPipelines();
}

PipelineManager::~PipelineManager()
{
    if(pipelineCache != nullptr)
        vkDestroyPipelineCache(Shared::device->device(), pipelineCache, nullptr);
}

void PipelineManager::reloadPipelines()
{
    vkDeviceWaitIdle(Shared::device->device());
    destroyPipelines();
    createPipelines();
}

void PipelineManager::createPipelines()
{
    // assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
    Console::log("Creating pipelines", "PipelineManager");
    currentID = 0;
    // PipelineConfigInfo pipelineConfig{};
    // GraphicsPipeline::defaultPipelineConfigInfo(pipelineConfig);
    int numShaders = Shared::shaders.size();
    for(std::unique_ptr<Shader>& shader : Shared::shaders)
    {    
        std::unique_ptr<GraphicsPipeline> graphicsPipeline = std::make_unique<GraphicsPipeline>(
            *shader,
            currentID++,
            pipelineCache
        );

        graphicsPipelines.push_back(std::move(graphicsPipeline));
    }
    Console::log("Pipelines created successfully", "PipelineManager");
}

void PipelineManager::destroyPipelines()
{
    // for(GraphicsPipeline &graphicsPipeline : graphicsPipelines)
    // {
    //     graphicsPipeline.();
    // }
    graphicsPipelines.clear();
}

void PipelineManager::createPipelineCache()
{
    VkPipelineCacheCreateInfo cacheCreateInfo{};
    cacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    if(vkCreatePipelineCache(Shared::device->device(), &cacheCreateInfo, nullptr, &pipelineCache) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline cache!");
    }
}
} // namespace graphics