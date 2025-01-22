#include <iostream>
#include <stdexcept>
#include <array>
#include <GLFW/glfw3.h>

#include "graphics.hpp"
#include "mesh.hpp"
#include "../core/input.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

namespace graphics
{

// struct ObjectUbo
// {

// };

struct PushConstants
{
    alignas(64) glm::mat4 model;
    // alignas(4) float time;
};

Graphics::Graphics(const std::string& name, const std::string& engine_name)
{
    globalPool = DescriptorPool::Builder(device)
        .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();
    
    init(name, engine_name);
}

Graphics::~Graphics()
{
    cleanup();
}

void Graphics::init(const std::string& name, const std::string& engine_name)
{
    if(!window.open)
    {
        throw std::runtime_error("Failed to open window");
    }

    cameraUboBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for(int i = 0; i < cameraUboBuffers.size(); i++)
    {
        cameraUboBuffers[i] = std::make_unique<Buffer>(
            device, 
            sizeof(CameraUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            device.properties.limits.minUniformBufferOffsetAlignment
        );
        cameraUboBuffers[i]->map();
    }

    globalSetLayout = DescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();
    
    globalDescriptorSets = std::vector<VkDescriptorSet>(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for(int i = 0; i < globalDescriptorSets.size(); i++)
    {
        VkDescriptorBufferInfo bufferInfo = cameraUboBuffers[i]->descriptorInfo();
        DescriptorWriter(*globalSetLayout, *globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(globalDescriptorSets[i]);
    }

    createPipelineLayout();
    createPipeline();

    std::cout << "Successfully initialized graphics" << std::endl;
}

void Graphics::cleanup()
{
    vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
    // vkDestroyInstance(instance, nullptr);
    window.close();
}


void Graphics::drawFrame(std::vector<core::GameObject>& gameObjects)
{
    if(camera != nullptr)
    {
        VkExtent2D extent = renderer.getExtent();
        camera->setAspectRatio(static_cast<float>(extent.width) / static_cast<float>(extent.height));
    }
    
    // std::cout << "Drawing Frame" << std::endl;

    if(VkCommandBuffer commandBuffer = renderer.startFrame())
    {
        uint32_t frameIndex = renderer.getFrameIndex();
        FrameInfo frameInfo{frameIndex, 0.0, commandBuffer, globalDescriptorSets[frameIndex]};

        CameraUbo cameraUbo{};
        cameraUbo.view = camera->getView();
        cameraUbo.proj = camera->getProjection();
        // std::cout << "View: " << glm::to_string(cameraUbo.view) << std::endl;
        // std::cout << "Proj: " << glm::to_string(cameraUbo.proj) << std::endl;
        cameraUboBuffers[frameIndex]->writeToBuffer(&cameraUbo);

        // std::cout << "Beginning Render Pass" << std::endl;
        renderer.beginRenderPass(commandBuffer);
        // std::cout << "Drawing Game Objects" << std::endl;
        renderGameObjects(frameInfo, gameObjects);
        // std::cout << "Ending Render Pass" << std::endl;
        renderer.endRenderPass(commandBuffer);
        // std::cout << "Ending Frame" << std::endl;
        renderer.endFrame();
    }
}


void Graphics::createPipelineLayout()
{
    std::cout << "Creating pipeline layout" << std::endl;

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstants);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout->getDescriptorSetLayout()};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if(vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create pipeline layout");
    }
}

void Graphics::createPipeline()
{
    assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
    std::cout << "Creating pipeline" << std::endl;
    PipelineConfigInfo pipelineConfig{};
    GraphicsPipeline::defaultPipelineConfigInfo(pipelineConfig);

    pipelineConfig.renderPass = renderer.getRenderPass();
    pipelineConfig.pipelineLayout = pipelineLayout;

    graphicsPipeline = std::make_unique<GraphicsPipeline>(
        device, 
        "internal/shaders/basicShader.vert.spv", 
        "internal/shaders/basicShader.frag.spv", 
        pipelineConfig
    );
}

void Graphics::renderGameObjects(FrameInfo& frameInfo, std::vector<core::GameObject>& gameObjects)
{
    VkCommandBuffer& commandBuffer = frameInfo.commandBuffer;
    graphicsPipeline->bind(commandBuffer);

    vkCmdBindDescriptorSets(
        commandBuffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        pipelineLayout, 
        0,
        1,
        &frameInfo.globalDescriptorSet, 
        0,
        nullptr
    );

    for(auto& obj : gameObjects)
    {
        PushConstants push{};
        push.model = obj.transform.getTransform();
        vkCmdPushConstants(
            commandBuffer, 
            pipelineLayout, 
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
            0, 
            sizeof(PushConstants), 
            &push
        );

        obj.mesh->bind(commandBuffer);
        obj.mesh->draw(commandBuffer);
    }
}

} // namespace graphics