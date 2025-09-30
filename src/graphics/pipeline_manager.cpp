#include "pipeline_manager.hpp"
#include "../core/console.hpp"

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

// void PipelineManager::renderObjects(FrameInfo& frameInfo, std::vector<core::GameObject>& gameObjects, VkCommandBuffer& commandBuffer)
// {
//     for(std::unique_ptr<GraphicsPipeline>& pipeline : graphicsPipelines)
//     {
//         pipeline->bind(commandBuffer);

//         std::vector<VkDescriptorSet> descriptorSets = { frameInfo.globalDescriptorSet };
//         // for(auto &m : materials)
//         // {
//         //     descriptorSets.push_back(m.getDescriptorSet());
//         //     std::cout << "Material Descriptor Set: " << m.getDescriptorSet() << std::endl;
//         // }

//         vkCmdBindDescriptorSets(
//             commandBuffer, 
//             VK_PIPELINE_BIND_POINT_GRAPHICS, 
//             pipelineLayout, 
//             0,
//             static_cast<uint32_t>(descriptorSets.size()),
//             descriptorSets.data(), 
//             0,
//             nullptr
//         );

//         for(core::GameObject& obj : gameObjects)
//         {
//             std::vector<VkDescriptorSet> localDescriptorSets = { Shared::materials[obj.materialID].getDescriptorSet() };
//             // uint32_t setIndex = 1 + Shared::materials[obj.materialID].getShaderID();
//             vkCmdBindDescriptorSets(
//                 commandBuffer, 
//                 VK_PIPELINE_BIND_POINT_GRAPHICS, 
//                 pipelineLayout, 
//                 1 + obj.materialID,
//                 1,
//                 localDescriptorSets.data(), 
//                 0,
//                 nullptr
//             );


//             PushConstants push{};
//             push.model = obj.transform.getTransform();
//             vkCmdPushConstants(
//                 commandBuffer, 
//                 pipelineLayout, 
//                 VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
//                 0, 
//                 sizeof(PushConstants), 
//                 &push
//             );

//             obj.mesh->bind(commandBuffer);
//             obj.mesh->draw(commandBuffer);
//         }
//     }
// }

} // namespace graphics