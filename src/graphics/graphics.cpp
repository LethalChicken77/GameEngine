#include <iostream>
#include <stdexcept>
#include <array>
#include <GLFW/glfw3.h>

#include "graphics.hpp"
#include "mesh.hpp"
#include "../core/input.hpp"
#include "material.hpp"

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
    containers.globalPool = DescriptorPool::Builder(device)
        .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();
    containers.materialPool = DescriptorPool::Builder(device)
        .setMaxSets(GR_MAX_MATERIAL_COUNT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, GR_MAX_MATERIAL_COUNT)
        .build();
    
    init(name, engine_name);
}

Graphics::~Graphics()
{
    cleanup();
}

void Graphics::init(const std::string& name, const std::string& engine_name)
{
    containers.device = &device;
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

    containers.globalSetLayout = DescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();
    
    containers.globalDescriptorSets = std::vector<VkDescriptorSet>(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for(int i = 0; i < containers.globalDescriptorSets.size(); i++)
    {
        VkDescriptorBufferInfo bufferInfo = cameraUboBuffers[i]->descriptorInfo();
        DescriptorWriter(*containers.globalSetLayout, *containers.globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(containers.globalDescriptorSets[i]);
    }

    loadShaders();
    loadMaterials();
    // TODO: Different pipeline for each pair of shaders
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
        FrameInfo frameInfo{frameIndex, 0.0, commandBuffer, containers.globalDescriptorSets[frameIndex]};

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

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{
        containers.globalSetLayout->getDescriptorSetLayout()
    };
    for(auto &m : materials)
    {
        descriptorSetLayouts.push_back(containers.materialSetLayout->getDescriptorSetLayout());
    }

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
    // PipelineConfigInfo pipelineConfig{};
    // GraphicsPipeline::defaultPipelineConfigInfo(pipelineConfig);
    PipelineConfigInfo &pipelineConfig = shaders[0]->getConfigInfo();

    pipelineConfig.renderPass = renderer.getRenderPass();
    pipelineConfig.pipelineLayout = pipelineLayout;

    graphicsPipeline = std::make_unique<GraphicsPipeline>(
        device,
        *shaders[0]
    );
}

void Graphics::loadShaders()
{
    std::cout << "Loading shaders\n";
    shaders.push_back(std::make_unique<Shader>(
        device,
        "internal/shaders/basicShader.vert.spv", 
        "internal/shaders/basicShader.frag.spv", 
        std::vector<Shader::ShaderInput>{
            {"color", Shader::ShaderInput::DataType::VEC3},
            {"ior", Shader::ShaderInput::DataType::VEC3},
            {"roughness", Shader::ShaderInput::DataType::FLOAT},
            {"metallic", Shader::ShaderInput::DataType::FLOAT}
        }
    ));
}

void Graphics::loadMaterials()
{
    std::cout << "Loading materials\n";
    materials.reserve(GR_MAX_MATERIAL_COUNT);
    containers.materialSetLayout = DescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();

    Material m1 = Material::instantiate(containers, shaders[0].get());
    m1.setValue("color", glm::vec3(1.0f, 0.0f, 0.0f));
    m1.setValue("ior", glm::vec3(1.5f, 1.5f, 1.5f));
    m1.setValue("roughness", 0.7f);
    m1.setValue("metallic", 0.f);
    m1.createShaderInputBuffer();
    materials.emplace_back(std::move(m1));

    Material m2 = Material::instantiate(containers, shaders[0].get());
    m2.setValue("color", glm::vec3(0.944f, 0.776f, 0.373f)); // Gold from physicallybased.info
    m2.setValue("ior", glm::vec3(0.18299f, 0.42108f, 1.37340f));
    m2.setValue("roughness", 0.4f);
    m2.setValue("metallic", 1.f);
    m2.createShaderInputBuffer();
    materials.emplace_back(std::move(m2));

    Material m3 = Material::instantiate(containers, shaders[0].get());
    m3.setValue("color", glm::vec3(0.8f, 0.2f, 0.1f));
    m3.setValue("ior", glm::vec3(1.5f, 1.5f, 1.5f));
    m3.setValue("roughness", 0.43f);
    m3.setValue("metallic", 0.f);
    m3.createShaderInputBuffer();
    materials.emplace_back(std::move(m3));
    

    containers.materialDescriptorSets = std::vector<VkDescriptorSet>(GR_MAX_MATERIAL_COUNT);
    containers.materialSetLayout = DescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();
    for(int i = 0; i < materials.size(); i++)
    {
        Material &m = materials[i];
        VkDescriptorBufferInfo bufferInfo = m.getBuffer()->descriptorInfo();
        DescriptorWriter(*containers.materialSetLayout, *containers.materialPool)
            .writeBuffer(0, &bufferInfo)
            .build(containers.materialDescriptorSets[i]);
    }
}

void Graphics::renderGameObjects(FrameInfo& frameInfo, std::vector<core::GameObject>& gameObjects)
{
    VkCommandBuffer& commandBuffer = frameInfo.commandBuffer;
    graphicsPipeline->bind(commandBuffer);

    std::vector<VkDescriptorSet> descriptorSets = { frameInfo.globalDescriptorSet };
    // for(auto &m : materials)
    // {
    //     descriptorSets.push_back(m.getDescriptorSet());
    //     std::cout << "Material Descriptor Set: " << m.getDescriptorSet() << std::endl;
    // }

    vkCmdBindDescriptorSets(
        commandBuffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        pipelineLayout, 
        0,
        static_cast<uint32_t>(descriptorSets.size()),
        descriptorSets.data(), 
        0,
        nullptr
    );

    materials[0].setValue("color", 
        glm::vec3((sin(glfwGetTime()) * 0.5f + 0.5f), 
        (sin(glfwGetTime() + glm::pi<float>() * 4.0f * 0.333333f) * 0.5f + 0.5f), 
        (sin(glfwGetTime() + glm::pi<float>() * 2.0f * 0.333333f) * 0.5f + 0.5f)));
    materials[0].createShaderInputBuffer();
    for(core::GameObject& obj : gameObjects)
    {
        std::vector<VkDescriptorSet> localDescriptorSets = { materials[obj.materialID].getDescriptorSet() };
        vkCmdBindDescriptorSets(
            commandBuffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS, 
            pipelineLayout, 
            1,
            1,
            localDescriptorSets.data(), 
            0,
            nullptr
        );


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