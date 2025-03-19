#include <iostream>
#include <stdexcept>
#include <array>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

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

Graphics::Graphics(const std::string& name, const std::string& engine_name)
{
    Descriptors::globalPool = DescriptorPool::Builder(device)
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
    std::cout << "Initializing Graphics\n";
    Shared::device = &device;
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

    Descriptors::globalSetLayout = DescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();
    
    Descriptors::globalDescriptorSets = std::vector<VkDescriptorSet>(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for(int i = 0; i < Descriptors::globalDescriptorSets.size(); i++)
    {
        VkDescriptorBufferInfo bufferInfo = cameraUboBuffers[i]->descriptorInfo();
        DescriptorWriter(*Descriptors::globalSetLayout, *Descriptors::globalPool)
            .writeBuffer(0, &bufferInfo)
            .build(Descriptors::globalDescriptorSets[i]);
    }

    loadShaders();
    loadMaterials();
    // TODO: Different pipeline for each pair of shaders
    createPipeline();

    std::cout << "Successfully initialized graphics" << std::endl;
}

void Graphics::cleanup()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    cameraUboBuffers.clear();
    Descriptors::globalPool.reset();
    Descriptors::globalSetLayout.reset();
    Descriptors::globalDescriptorSets.clear();
    Descriptors::imguiPool.reset();
    graphicsPipeline.reset();
    shaders.clear();
    Shared::materials.clear();
    waitForDevice();
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
        FrameInfo frameInfo{frameIndex, 0.0, commandBuffer, Descriptors::globalDescriptorSets[frameIndex]};

        CameraUbo cameraUbo{};
        cameraUbo.view = camera->getView();
        cameraUbo.proj = camera->getProjection();
        // std::cout << "View: " << glm::to_string(cameraUbo.view) << std::endl;
        // std::cout << "Proj: " << glm::to_string(cameraUbo.proj) << std::endl;
        cameraUboBuffers[frameIndex]->writeToBuffer(&cameraUbo);

        // Objects render pass
        renderer.beginRenderPass(commandBuffer);
        renderGameObjects(frameInfo, gameObjects);
        // renderer.endRenderPass(commandBuffer);

        // ImGui render pass
        // renderer.beginRenderPass(commandBuffer);
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
        renderer.endRenderPass(commandBuffer);

        renderer.endFrame();
    }
}

void Graphics::createPipeline()
{
    // assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
    std::cout << "Creating pipeline" << std::endl;
    // PipelineConfigInfo pipelineConfig{};
    // GraphicsPipeline::defaultPipelineConfigInfo(pipelineConfig);
    PipelineConfigInfo &pipelineConfig = shaders[0]->getConfigInfo();

    pipelineConfig.renderPass = renderer.getRenderPass();
    // pipelineConfig.pipelineLayout = pipelineLayout;

    graphicsPipeline = std::make_unique<GraphicsPipeline>(
        device,
        *shaders[0]
    );
}

void Graphics::loadShaders()
{
    std::cout << "Loading shaders\n";
    shaders.push_back(std::make_unique<Shader>(
        "internal/shaders/basicShader.vert.spv", 
        "internal/shaders/basicShader.frag.spv", 
        std::vector<ShaderInput>{
            {"color", ShaderInput::DataType::VEC3},
            {"ior", ShaderInput::DataType::VEC3},
            {"roughness", ShaderInput::DataType::FLOAT},
            {"metallic", ShaderInput::DataType::FLOAT}
        },
        3
    ));
}

void Graphics::loadMaterials()
{
    std::cout << "Loading materials\n";
    Shared::materials.reserve(GR_MAX_MATERIAL_COUNT);

    Material m1 = Material::instantiate(shaders[0].get());
    // m1.setValue("color", glm::vec3(0.1f, 0.3f, 0.05f));
    m1.setValue("color", glm::vec3(1.f, 1.f, 1.f));
    m1.setValue("ior", glm::vec3(1.5f, 1.5f, 1.5f));
    m1.setValue("roughness", 0.95f);
    m1.setValue("metallic", 0.f);
    textures.push_back(Texture::loadFromFile("./internal/textures/rocky_terrain_02/rocky_terrain_02_diff_4k.jpg"));
    textures.push_back(Texture::loadFromFile("./internal/textures/rocky_terrain_02/rocky_terrain_02_rough_4k.png"));
    textures.push_back(Texture::loadFromFile("./internal/textures/rocky_terrain_02/rocky_terrain_02_nor_gl_4k.png"));

    int heightmapResolution = 1024;
    std::shared_ptr heightmapTexture = std::make_shared<Texture>(heightmapResolution, heightmapResolution);
    
    heightmapTexture->createTexture();

    m1.createShaderInputBuffer();
    m1.setTexture(0, textures[0]);
    m1.setTexture(1, textures[1]);
    m1.setTexture(2, textures[2]);
    m1.updateDescriptorSet();
    Shared::materials.emplace_back(std::move(m1));

    // Material m2 = Material::instantiate(shaders[0].get());
    // // m2.setValue("color", glm::vec3(0.944f, 0.776f, 0.373f)); // Gold from physicallybased.info
    // m2.setValue("color", glm::vec3(1.f, 1.f, 1.f));
    // // m2.setValue("ior", glm::vec3(0.18299f, 0.42108f, 1.37340f)); // Gold
    // m2.setValue("ior", glm::vec3(0.159f, 0.145f, 0.135f)); // Silver
    // m2.setValue("roughness", 0.4f);
    // m2.setValue("metallic", 1.f);
    // m2.createShaderInputBuffer();
    // Shared::materials.emplace_back(std::move(m2));

    // Material m3 = Material::instantiate(shaders[0].get());
    // m3.setValue("color", glm::vec3(0.8f, 0.2f, 0.1f));
    // m3.setValue("ior", glm::vec3(1.5f, 1.5f, 1.5f));
    // m3.setValue("roughness", 0.43f);
    // m3.setValue("metallic", 0.f);
    // m3.createShaderInputBuffer();
    // Shared::materials.emplace_back(std::move(m3));
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
        graphicsPipeline->getPipelineLayout(), 
        0,
        static_cast<uint32_t>(descriptorSets.size()),
        descriptorSets.data(), 
        0,
        nullptr
    );

    for(core::GameObject& obj : gameObjects)
    {
        std::vector<VkDescriptorSet> localDescriptorSets = { Shared::materials[obj.materialID].getDescriptorSet() };
        vkCmdBindDescriptorSets(
            commandBuffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS, 
            graphicsPipeline->getPipelineLayout(), 
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
            graphicsPipeline->getPipelineLayout(), 
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
            0, 
            sizeof(PushConstants), 
            &push
        );

        obj.mesh->bind(commandBuffer);
        obj.mesh->draw(commandBuffer);
    }
}

void Graphics::graphicsInitImgui()
{
    // containers.imguiDescriptorSets = std::vector<VkDescriptorSet>(SwapChain::MAX_FRAMES_IN_FLIGHT);
    Descriptors::imguiPool = DescriptorPool::Builder(device)
        .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
        .build();

    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = Shared::instance;
    initInfo.PhysicalDevice = device.getPhysicalDevice();
    initInfo.Device = device.device();
    initInfo.QueueFamily = device.findPhysicalQueueFamilies().graphicsFamily;
    initInfo.Queue = device.graphicsQueue();
    initInfo.PipelineCache = VK_NULL_HANDLE;
    initInfo.DescriptorPool = Descriptors::imguiPool->getPool();
    initInfo.RenderPass = renderer.getRenderPass();
    initInfo.Allocator = nullptr;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = SwapChain::MAX_FRAMES_IN_FLIGHT;
    initInfo.CheckVkResultFn = [](VkResult err) {
        if (err != VK_SUCCESS) {
            fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        }
    };
    ImGui_ImplVulkan_Init(&initInfo);
}
} // namespace graphics