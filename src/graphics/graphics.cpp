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
#include "../core/random.hpp"
#include "../procedural/noise.hpp"

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
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
        .build();

    Descriptors::cameraPool = DescriptorPool::Builder(device)
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
    Console::log("Initializing graphics module", "Graphics");
    Shared::device = &device;
    if(!window.open)
    {
        throw std::runtime_error("Failed to open window");
    }

    Console::log("Creating global UBO", "Graphics");
    // Global data
    globalUboBuffer = std::make_unique<Buffer>(
        device,
        sizeof(GlobalUbo),
        1,
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        device.properties.limits.minUniformBufferOffsetAlignment
    );
    globalUboBuffer->map();
    Descriptors::globalSetLayout = DescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();
    
    
    VkDescriptorBufferInfo bufferInfo = globalUboBuffer->descriptorInfo();
    DescriptorWriter(*Descriptors::globalSetLayout, *Descriptors::globalPool)
        .writeBuffer(0, &bufferInfo)
        .build(Descriptors::globalDescriptorSet);

    // Camera
    Console::log("Creating camera UBO", "Graphics");
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

    Descriptors::cameraSetLayout = DescriptorSetLayout::Builder(device)
        .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .build();
    
    Descriptors::cameraDescriptorSets = std::vector<VkDescriptorSet>(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for(int i = 0; i < Descriptors::cameraDescriptorSets.size(); i++)
    {
        VkDescriptorBufferInfo bufferInfo = cameraUboBuffers[i]->descriptorInfo();
        DescriptorWriter(*Descriptors::cameraSetLayout, *Descriptors::cameraPool)
            .writeBuffer(0, &bufferInfo)
            .build(Descriptors::cameraDescriptorSets[i]);
    }

    loadTextures();
    loadShaders();
    loadMaterials();
    pipelineManager = std::make_unique<PipelineManager>(renderer);
    // pipelineManager->createPipelines();

    Console::log("Successfully initialized graphics", "Graphics");
}

void Graphics::cleanup()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    globalUboBuffer.reset();
    cameraUboBuffers.clear();
    Descriptors::globalPool.reset();
    Descriptors::globalSetLayout.reset();
    Descriptors::cameraPool.reset();
    Descriptors::cameraSetLayout.reset();
    Descriptors::imguiPool.reset();
    pipelineManager->destroyPipelines();
    // graphicsPipeline.reset();
    Shared::shaders.clear();
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
        FrameInfo frameInfo{frameIndex, 0.0, commandBuffer, Descriptors::globalDescriptorSet, Descriptors::cameraDescriptorSets[frameIndex]};

        GlobalUbo globalUbo{};
        globalUbo.lights[0] = {glm::vec3(1, 1, 1), LightType::DIRECTIONAL, glm::vec3(1.0, 1.0, 1.0), 9.0};
        globalUbo.lights[1] = {glm::vec3(4, 0, 0), LightType::POINT, glm::vec3(1.0, 0.8, 0.1), 30.0};
        globalUbo.lights[2] = {glm::vec3(0, 4, -4), LightType::POINT, glm::vec3(0.5, 1.0, 0.1), 10.0};
        globalUbo.lights[3] = {glm::vec3(-4, 0, 2), LightType::POINT, glm::vec3(0.9, 0.2, 1.0), 10.0};
        globalUbo.numLights = 4;
        globalUbo.ambient = glm::vec3(0.04, 0.08, 0.2);
        // globalUbo.ambient = glm::vec3(1, 1, 1);
        globalUboBuffer->writeToBuffer(&globalUbo);

        CameraUbo cameraUbo{};
        cameraUbo.view = camera->getView();
        cameraUbo.invView = glm::inverse(camera->getView());
        cameraUbo.proj = camera->getProjection();
        cameraUbo.viewProj = camera->getViewProjection();
        // std::cout << "View: " << glm::to_string(cameraUbo.view) << std::endl;
        // std::cout << "Proj: " << glm::to_string(cameraUbo.proj) << std::endl;
        cameraUboBuffers[frameIndex]->writeToBuffer(&cameraUbo);

        // Objects render pass
        renderer.beginRenderPass(renderer.getSCRenderPass(), renderer.getSCFrameBuffer(), renderer.getExtent());
        renderGameObjects(frameInfo, gameObjects);

        // renderer.endRenderPass(commandBuffer);
        
        // renderer.beginRenderPass(commandBuffer);
        
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
        renderer.endRenderPass();

        renderer.endFrame();
    }
}


void Graphics::loadTextures()
{
    Console::log("Loading textures", "Graphics");
    textures.push_back(Texture::loadFromFile("./internal/textures/worn_tile_floor/worn_tile_floor_diff_1k.jpg"));
    textures.push_back(Texture::loadFromFileEXR("./internal/textures/worn_tile_floor/worn_tile_floor_rough_1k.exr"));
    textures.push_back(Texture::loadFromFileEXR("./internal/textures/defaults/default_metal.exr"));
    textures.push_back(Texture::loadFromFileEXR("./internal/textures/defaults/default_spec.exr"));
    textures.push_back(Texture::loadFromFileEXR("./internal/textures/worn_tile_floor/worn_tile_floor_nor_gl_1k.exr"));
    // textures.push_back(Texture::loadFromFile("./internal/textures/TestNormalMap.png", VK_FORMAT_R8G8B8A8_UNORM));
    // textures.push_back(Texture::loadFromFile("./internal/textures/rusted_metal/rusty_metal_04_diff_1k.jpg"));
    // textures.push_back(Texture::loadFromFileEXR("./internal/textures/rusted_metal/rusty_metal_04_rough_1k.exr"));
    // textures.push_back(Texture::loadFromFileEXR("./internal/textures/rusted_metal/rusty_metal_04_metal_1k.exr"));
    // textures.push_back(Texture::loadFromFileEXR("./internal/textures/rusted_metal/rusty_metal_04_spec_1k.exr"));
    // textures.push_back(Texture::loadFromFileEXR("./internal/textures/rusted_metal/rusty_metal_04_nor_gl_1k.exr"));
    // textures.push_back(Texture::loadFromFile("./internal/textures/TestNormalMap.png"));
    textures.push_back(Texture::loadFromFile("./internal/textures/skybox_blurred.png"));
    // textures.push_back(Texture::loadFromFile("./internal/textures/rocky_terrain_02/rocky_terrain_02_diff_512.jpg"));
    // textures.push_back(Texture::loadFromFile("./internal/textures/rocky_terrain_02/rocky_terrain_02_rough_512.png"));
    // textures.push_back(Texture::loadFromFile("./internal/textures/rocky_terrain_02/rocky_terrain_02_nor_gl_512.png"));
    // textures.push_back(Texture::loadFromFile("./internal/textures/rocky_terrain_02/rocky_terrain_02_diff_4k.jpg"));
    // textures.push_back(Texture::loadFromFile("./internal/textures/rocky_terrain_02/rocky_terrain_02_rough_4k.png"));
    // textures.push_back(Texture::loadFromFile("./internal/textures/rocky_terrain_02/rocky_terrain_02_nor_gl_4k.png"));

    // textures.push_back(Texture::loadFromFile("./internal/textures/rock_face/rock_face_diff_512.jpg"));
    // textures.push_back(Texture::loadFromFile("./internal/textures/rock_face/rock_face_rough_512.png"));
    // textures.push_back(Texture::loadFromFile("./internal/textures/rock_face/rock_face_nor_gl_512.png"));
    // textures.push_back(Texture::loadFromFile("./internal/textures/rock_face/rock_face_diff_4k.jpg"));
    // textures.push_back(Texture::loadFromFile("./internal/textures/rock_face/rock_face_rough_4k.png"));
    // textures.push_back(Texture::loadFromFile("./internal/textures/rock_face/rock_face_nor_gl_4k.png"));
    
    // textures.push_back(heightmapTexture);
}

void Graphics::loadShaders()
{
    Console::log("Loading shaders", "Graphics");
    // Shared::shaders.push_back(std::make_unique<Shader>(
    //     "internal/shaders/basicShader.vert.spv", 
    //     "internal/shaders/basicShader.frag.spv", 
    //     std::vector<ShaderInput>{
    //         {"color", ShaderInput::DataType::VEC3},
    //         {"ior", ShaderInput::DataType::VEC3},
    //         {"roughness", ShaderInput::DataType::FLOAT},
    //         {"metallic", ShaderInput::DataType::FLOAT}
    //     },
    //     6
    // ));
    Shared::shaders.push_back(std::make_unique<Shader>(
        "internal/shaders/skybox.slang", 
        "internal/shaders/skybox.slang", 
        std::vector<ShaderInput>{
            {"color", ShaderInput::DataType::COLOR}
        },
        0
    ));
    
    Shared::shaders[0]->configInfo.depthStencilInfo.depthTestEnable = VK_FALSE;
    Shared::shaders[0]->configInfo.depthStencilInfo.depthWriteEnable = VK_FALSE;
    Shared::shaders[0]->reloadShader();

    Shared::shaders.push_back(std::make_unique<Shader>(
        "internal/shaders/basicShader.slang",
        "internal/shaders/basicShader.slang",
        std::vector<ShaderInput>{
            {"color", ShaderInput::DataType::COLOR},
            {"roughness", ShaderInput::DataType::FLOAT},
            {"metallic", ShaderInput::DataType::FLOAT}
        },
        0
    ));

    Shared::shaders.push_back(std::make_unique<Shader>(
        "internal/shaders/wireframe.slang", 
        "internal/shaders/wireframe.slang", 
        std::vector<ShaderInput>{
            {"color", ShaderInput::DataType::COLOR}
        },
        0
    ));
    // Shared::shaders[0]->configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
    Shared::shaders[2]->configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
    Shared::shaders[2]->configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    // Shared::shaders[1]->configInfo.depthStencilInfo.depthTestEnable = VK_FALSE;
    Shared::shaders[2]->reloadShader();

    Shared::shaders.push_back(std::make_unique<Shader>(
        "internal/shaders/PBR.slang", 
        "internal/shaders/PBR.slang", 
        std::vector<ShaderInput>{
            {"color", ShaderInput::DataType::COLOR},
            {"normalMapStrength", ShaderInput::DataType::FLOAT}
        },
        6
    ));
}

void Graphics::loadMaterials()
{
    Console::log("Loading materials", "Graphics");
    Shared::materials.reserve(GR_MAX_MATERIAL_COUNT);
    
    Material skybox = Material::instantiate(Shared::shaders[0].get());
    // m1.setValue("color", glm::vec3(0.1f, 0.3f, 0.05f));
    skybox.setValue("color", Color(0.f, 0.f, 0.f));
    skybox.createShaderInputBuffer();
    skybox.createDescriptorSet();
    Shared::materials.emplace_back(std::move(skybox));

    Material m1 = Material::instantiate(Shared::shaders[1].get());
    // m1.setValue("color", glm::vec3(0.1f, 0.3f, 0.05f));
    m1.setValue("color", Color(1.f, 0.8f, 0.3f));
    m1.setValue("roughness", 0.4f);
    m1.setValue("metallic", 0.0f);
    m1.createShaderInputBuffer();
    m1.createDescriptorSet();
    Shared::materials.emplace_back(std::move(m1)); // Should probably be done in instantiate

    Material m3 = Material::instantiate(Shared::shaders[2].get());
    // m1.setValue("color", glm::vec3(0.1f, 0.3f, 0.05f));
    m3.setValue("color", Color(0.f, 0.f, 0.f));
    m3.createShaderInputBuffer();
    m3.createDescriptorSet();
    Shared::materials.emplace_back(std::move(m3)); // Should probably be done in instantiate

    Material m4 = Material::instantiate(Shared::shaders[3].get());
    m4.setValue("color", Color(1.f, 1.f, 1.f));
    m4.setValue("normalMapStrength", 1.0f);
    m4.setTexture(0, textures[0]); // Albedo
    m4.setTexture(1, textures[1]); // Roughness
    m4.setTexture(2, textures[2]); // Metallic
    m4.setTexture(3, textures[3]); // Specular
    m4.setTexture(4, textures[4]); // Normal
    m4.setTexture(5, textures[5]); // Skybox (TEMPORARY)
    
    m4.createShaderInputBuffer();
    m4.createDescriptorSet();
    Shared::materials.emplace_back(std::move(m4));
}

void Graphics::renderGameObjects(FrameInfo& frameInfo, std::vector<core::GameObject>& gameObjects)
{
    VkCommandBuffer& commandBuffer = frameInfo.commandBuffer;

    // pipelineManager.renderObjects(frameInfo, gameObjects, commandBuffer);
    GraphicsPipeline* prevPipeline = nullptr;
    std::vector<VkDescriptorSet> localDescriptorSets;
    VkPipelineLayout pipelineLayout;
    Material::id_t prevMaterial = UINT64_MAX;
    for(core::GameObject& obj : gameObjects)
    {
        const Shader* shader = Shared::materials[obj.materialID].getShader();
        GraphicsPipeline* pipeline = shader->getPipeline();
        uint32_t setIndex = pipeline->getID() + 1;
        if(pipeline != prevPipeline) // Bind camera and global data
        {
            pipeline->bind(commandBuffer);
            pipelineLayout = pipeline->getPipelineLayout();

            std::vector<VkDescriptorSet> descriptorSets = { frameInfo.cameraDescriptorSet };

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


            descriptorSets = { frameInfo.globalDescriptorSet };

            vkCmdBindDescriptorSets(
                commandBuffer, 
                VK_PIPELINE_BIND_POINT_GRAPHICS, 
                pipelineLayout, 
                1,
                static_cast<uint32_t>(descriptorSets.size()),
                descriptorSets.data(), 
                0,
                nullptr
            );

            prevPipeline = pipeline;
        }
        localDescriptorSets = { Shared::materials[obj.materialID].getDescriptorSet() };

        if(prevMaterial != obj.materialID) // Bind material info if changed
        {
            vkCmdBindDescriptorSets(
                commandBuffer, 
                VK_PIPELINE_BIND_POINT_GRAPHICS, 
                pipelineLayout, 
                2,
                1,
                localDescriptorSets.data(), 
                0,
                nullptr
            );
            prevMaterial = obj.materialID;
        }

        PushConstants push{};
        push.model = obj.transform.getTransform();
        vkCmdPushConstants(
            commandBuffer, 
            pipeline->getPipelineLayout(), 
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
    initInfo.RenderPass = renderer.getSCRenderPass();
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

void Graphics::reloadShaders()
{
    std::cout << "Reloading Shaders" << std::endl;
    for(std::unique_ptr<Shader>& shader : Shared::shaders)
    {
        shader->reloadShader();
    }

    pipelineManager->reloadPipelines();
}

} // namespace graphics