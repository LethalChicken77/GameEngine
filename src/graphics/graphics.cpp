#include <iostream>
#include <stdexcept>
#include <array>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_vulkan.h"
#include "imgui_impl_glfw.h"

#include "graphics.hpp"
#include "graphics_mesh.hpp"
#include "core/input.hpp"
#include "material.hpp"
#include "core/random.hpp"
#include "procedural/noise.hpp"

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


Graphics::~Graphics()
{
    // cleanup();
}

void Graphics::init(const std::string& name, const std::string& engine_name)
{
    Descriptors::globalPool = DescriptorPool::Builder(device)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1)
        .build();

    Descriptors::cameraPool = DescriptorPool::Builder(device)
        .setMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
        .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .build();
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

    createRenderPasses();
    loadTextures();
    loadShaders();
    loadMaterials();
    skyboxMesh = core::Mesh::createSkybox(100);
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


void Graphics::drawFrame()
{
    VkExtent2D extent = renderer.getExtent();
    if(extent.width <= 0 || extent.height <= 0) return; // Don't draw frame if minimized
    if(camera != nullptr)
    {
        camera->setAspectRatio(static_cast<float>(extent.width) / static_cast<float>(extent.height));
    }
    
    // std::cout << "Drawing Frame" << std::endl;
    std::vector<VkDescriptorSet> localDescriptorSets;
    if(VkCommandBuffer commandBuffer = renderer.startFrame())
    {
        uint32_t frameIndex = renderer.getFrameIndex();
        FrameInfo frameInfo{frameIndex, 0.0, commandBuffer, Descriptors::globalDescriptorSet, Descriptors::cameraDescriptorSets[frameIndex]};

        GlobalUbo globalUbo{};
        globalUbo.lights[0] = {glm::vec3(1, 1, 1), LightType::DIRECTIONAL, glm::vec3(1.0, 1.0, 1.0), 6.0};
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

        idBufferRenderPass->resetLayouts();
        sceneRenderPass->resetLayouts();
        imguiRenderPass->resetLayouts();
        finalRenderPass->resetLayouts();
        float frameScale = 1.0;
        VkExtent2D scaledExtent{
            static_cast<uint32_t>(extent.width * frameScale), 
            static_cast<uint32_t>(extent.height * frameScale)};
        if(sceneRenderPass->getExtent().width != scaledExtent.width || sceneRenderPass->getExtent().height != scaledExtent.height)
        {
            sceneRenderPass->create(scaledExtent);
        }
        if(imguiRenderPass->getExtent().width != extent.width || imguiRenderPass->getExtent().height != extent.height)
        {
            idBufferRenderPass->create(extent); // Maybe render at low resolution
            imguiRenderPass->create(extent);
            finalRenderPass->create(extent);
        }
        // Object IDs render pass
        renderer.beginRenderPass(idBufferRenderPass->getRenderPass(), idBufferRenderPass->getFrameBuffer(), idBufferRenderPass->getExtent(), VkClearColorValue{-1, 0, 0, 0});
        renderGameObjectIDs(frameInfo);
        renderer.endRenderPass();
        
        idTexture = idBufferRenderPass->getColorTexture().get();
        // idTexture->transitionImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, commandBuffer);
        // Objects render pass
        renderer.beginRenderPass(sceneRenderPass->getRenderPass(), sceneRenderPass->getFrameBuffer(), sceneRenderPass->getExtent(), defaultClearColor);
        renderMeshes(frameInfo, sceneRenderQueue);
        renderer.endRenderPass();

        std::unique_ptr<Texture> &colorTexture = sceneRenderPass->getColorTexture();
        std::unique_ptr<Texture> &depthTexture = sceneRenderPass->getDepthTexture();
        colorTexture->transitionImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, commandBuffer);
        depthTexture->transitionImageLayout(VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL, commandBuffer);
        ppMaterial->setTexture(0, colorTexture.get());
        ppMaterial->setTexture(1, depthTexture.get());
        ppMaterial->createDescriptorSet();

        
        // renderer.beginRenderPass(outlineBaseRenderPass->getRenderPass(), outlineBaseRenderPass->getFrameBuffer(), outlineBaseRenderPass->getExtent(), {0,0,0,1});
        // renderMeshes(frameInfo, outlineRenderQueue);
        // renderer.endRenderPass();

        renderer.beginRenderPass(imguiRenderPass->getRenderPass(), imguiRenderPass->getFrameBuffer(), imguiRenderPass->getExtent(), VkClearColorValue{0.0f, 0.0f, 0.0f, 0.0f});
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
        renderer.endRenderPass();

        std::unique_ptr<Texture> &imguiTexture = imguiRenderPass->getColorTexture();
        imguiTexture->transitionImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, commandBuffer);
        imguiMaterial->setTexture(0, imguiTexture.get());
        imguiMaterial->createDescriptorSet();

        renderer.beginRenderPass(finalRenderPass->getRenderPass(), finalRenderPass->getFrameBuffer(), finalRenderPass->getExtent(), defaultClearColor);
        pipelineManager->getPipeline(0)->bind(commandBuffer); // Post-processing pipeline

        localDescriptorSets = { ppMaterial->getDescriptorSet() };
        vkCmdBindDescriptorSets(
            commandBuffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS, 
            pipelineManager->getPipeline(0)->getPipelineLayout(), 
            0,
            1,
            localDescriptorSets.data(), 
            0,
            nullptr
        );

        // Draw 6 vertices (full-screen quad)
        vkCmdDraw(commandBuffer, 6, 1, 0, 0);

        pipelineManager->getPipeline(1)->bind(commandBuffer); // Post-processing pipeline
        localDescriptorSets = { imguiMaterial->getDescriptorSet() };
        vkCmdBindDescriptorSets(
            commandBuffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS, 
            pipelineManager->getPipeline(1)->getPipelineLayout(), 
            0,
            1,
            localDescriptorSets.data(), 
            0,
            nullptr
        );

        vkCmdDraw(commandBuffer, 6, 1, 0, 0);
        renderer.endRenderPass();

        std::unique_ptr<Texture> &outputTexture = finalRenderPass->getColorTexture();
        outputTexture->transitionImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, commandBuffer);
        outputMaterial->setTexture(0, outputTexture.get());
        outputMaterial->createDescriptorSet();

        renderer.beginRenderPass(renderer.getSCRenderPass(), renderer.getSCFrameBuffer(), extent, defaultClearColor);
        outputMaterial->getShader()->getPipeline()->bind(commandBuffer); // Post-processing pipeline
        localDescriptorSets = { outputMaterial->getDescriptorSet() };
        vkCmdBindDescriptorSets(
            commandBuffer, 
            VK_PIPELINE_BIND_POINT_GRAPHICS, 
            outputMaterial->getShader()->getPipeline()->getPipelineLayout(), 
            0,
            1,
            localDescriptorSets.data(), 
            0,
            nullptr
        );

        vkCmdDraw(commandBuffer, 6, 1, 0, 0);
        renderer.endRenderPass();

        renderer.endFrame();
    }
    vkDeviceWaitIdle(Shared::device->device());
    sceneRenderQueue.clear();
}

void Graphics::createRenderPasses()
{
    Console::log("Creating ID buffer render pass", "Graphics");
    idBufferRenderPass = std::make_unique<RenderPass>(renderer.getExtent());
    idBufferRenderPass->addColorAttachment(VK_FORMAT_R32_SINT);
    idBufferRenderPass->addDepthAttachment();
    idBufferRenderPass->create(renderer.getExtent());

    Console::log("Creating scene render pass", "Graphics");
    sceneRenderPass = std::make_unique<RenderPass>(renderer.getExtent());
    sceneRenderPass->addColorAttachment(VK_FORMAT_R16G16B16A16_SFLOAT);
    sceneRenderPass->addDepthAttachment();
    sceneRenderPass->create(renderer.getExtent());

    Console::log("Creating outline base render pass", "Graphics");
    outlineBaseRenderPass = std::make_unique<RenderPass>(renderer.getExtent());
    SamplerProperties outlineBaseSamplerProperties = SamplerProperties::getDefaultProperties();
    outlineBaseSamplerProperties.addressMode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
    outlineBaseRenderPass->addColorAttachment(outlineBaseSamplerProperties, VK_FORMAT_R16G16B16A16_SFLOAT);
    // outlineRenderPass->addDepthAttachment(); 
    outlineBaseRenderPass->create(renderer.getExtent());
    
    Console::log("Creating outline render pass", "Graphics");
    outlineRenderPass = std::make_unique<RenderPass>(renderer.getExtent());
    outlineRenderPass->addColorAttachment(VK_FORMAT_B8G8R8A8_SRGB);
    outlineRenderPass->addDepthAttachment();
    outlineRenderPass->create(renderer.getExtent());

    Console::log("Creating ImGui render pass", "Graphics");
    imguiRenderPass = std::make_unique<RenderPass>(renderer.getExtent());
    imguiRenderPass->addColorAttachment(VK_FORMAT_B8G8R8A8_SRGB);
    imguiRenderPass->addDepthAttachment();
    imguiRenderPass->create(renderer.getExtent());
    
    Console::log("Creating Final render pass", "Graphics");
    finalRenderPass = std::make_unique<RenderPass>(renderer.getExtent());
    finalRenderPass->addColorAttachment(VK_FORMAT_B8G8R8A8_SRGB); // Maybe change to UNORM
    finalRenderPass->addDepthAttachment();
    finalRenderPass->create(renderer.getExtent());
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
    PipelineConfigInfo ppConfigInfo = Shader::getDefaultConfigInfo();
    ppConfigInfo.pipelineType = POST_PROCESSING;
    ppConfigInfo.renderPass = &renderer.getSCRenderPass();
    Shared::shaders.push_back(std::make_unique<Shader>(
        "internal/shaders/post_processing/postProcessing.slang", //0
        "internal/shaders/post_processing/postProcessing.slang", 
        std::vector<ShaderInput>{
            {"exposure", ShaderInput::DataType::FLOAT},
            {"gamma", ShaderInput::DataType::FLOAT}
        },
        2,
        ppConfigInfo
    ));

    PipelineConfigInfo imguiConfigInfo = Shader::getDefaultTransparentConfigInfo();
    imguiConfigInfo.pipelineType = POST_PROCESSING;
    imguiConfigInfo.depthStencilInfo.depthTestEnable = VK_FALSE;
    imguiConfigInfo.depthStencilInfo.depthWriteEnable = VK_FALSE;
    imguiConfigInfo.renderPass = &imguiRenderPass->getRenderPass();
    Shared::shaders.push_back(std::make_unique<Shader>(
        "internal/shaders/post_processing/overlay.slang", //1
        "internal/shaders/post_processing/overlay.slang", 
        std::vector<ShaderInput>{
            {"doSRGBTransform", ShaderInput::DataType::BOOL}
        },
        1,
        imguiConfigInfo
    ));

    PipelineConfigInfo outlineConfigInfo = Shader::getDefaultTransparentConfigInfo();
    outlineConfigInfo.pipelineType = POST_PROCESSING;
    outlineConfigInfo.depthStencilInfo.depthTestEnable = VK_FALSE;
    outlineConfigInfo.depthStencilInfo.depthWriteEnable = VK_FALSE;
    outlineConfigInfo.renderPass = &imguiRenderPass->getRenderPass();
    Shared::shaders.push_back(std::make_unique<Shader>(
        "internal/shaders/post_processing/outline.slang", //2
        "internal/shaders/post_processing/outline.slang", 
        std::vector<ShaderInput>{
            {"filler", ShaderInput::DataType::FLOAT} // TODO: Allow shaders with no parameters (DUH)
        },
        1,
        imguiConfigInfo
    ));

    PipelineConfigInfo idBufferConfigInfo = Shader::getDefaultConfigInfo();
    idBufferConfigInfo.pipelineType = ID_BUFFER;
    idBufferConfigInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE; // Allow selecting meshes via backfaces
    idBufferConfigInfo.renderPass = &idBufferRenderPass->getRenderPass();
    Shared::shaders.push_back(std::make_unique<Shader>(
        "internal/shaders/id_buffer.slang", //3
        "internal/shaders/id_buffer.slang", 
        std::vector<ShaderInput>{
            {"objectID", ShaderInput::DataType::INT}
        },
        0,
        idBufferConfigInfo
    ));

    PipelineConfigInfo outlineBaseConfigInfo = Shader::getDefaultConfigInfo();
    outlineBaseConfigInfo.pipelineType = STANDARD;
    outlineBaseConfigInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE; // Allow selecting meshes via backfaces
    outlineBaseConfigInfo.depthStencilInfo.depthTestEnable = VK_FALSE;
    outlineBaseConfigInfo.depthStencilInfo.depthWriteEnable = VK_FALSE;
    outlineBaseConfigInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
    outlineBaseConfigInfo.renderPass = &outlineRenderPass->getRenderPass();
    Shared::shaders.push_back(std::make_unique<Shader>(
        "internal/shaders/outlineBase.slang", //4
        "internal/shaders/outlineBase.slang", 
        std::vector<ShaderInput>{
            {"filler", ShaderInput::DataType::FLOAT}
        },
        0,
        idBufferConfigInfo
    ));

    PipelineConfigInfo skyboxConfigInfo = Shader::getDefaultConfigInfo();
    // skyboxConfigInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
    // skyboxConfigInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_NEVER;
    skyboxConfigInfo.depthStencilInfo.depthWriteEnable = VK_FALSE;
    skyboxConfigInfo.renderPass = &sceneRenderPass->getRenderPass();
    Shared::shaders.push_back(std::make_unique<Shader>(
        "internal/shaders/skybox.slang", //5
        "internal/shaders/skybox.slang", 
        std::vector<ShaderInput>{
            {"color", ShaderInput::DataType::COLOR}
        },
        0,
        skyboxConfigInfo
    ));
    

    Shared::shaders.push_back(std::make_unique<Shader>(
        "internal/shaders/basicShader.slang", //6
        "internal/shaders/basicShader.slang",
        std::vector<ShaderInput>{
            {"color", ShaderInput::DataType::COLOR},
            {"roughness", ShaderInput::DataType::FLOAT},
            {"metallic", ShaderInput::DataType::FLOAT}
        },
        0,
        &sceneRenderPass->getRenderPass()
    ));

    PipelineConfigInfo wireframeConfigInfo = Shader::getDefaultConfigInfo();
    wireframeConfigInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_LINE;
    wireframeConfigInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE;
    wireframeConfigInfo.renderPass = &sceneRenderPass->getRenderPass();
    Shared::shaders.push_back(std::make_unique<Shader>(
        "internal/shaders/wireframe.slang", //7
        "internal/shaders/wireframe.slang", 
        std::vector<ShaderInput>{
            {"color", ShaderInput::DataType::COLOR}
        },
        0,
        wireframeConfigInfo
    ));

    Shared::shaders.push_back(std::make_unique<Shader>(
        "internal/shaders/PBR.slang", //8
        "internal/shaders/PBR.slang", 
        std::vector<ShaderInput>{
            {"color", ShaderInput::DataType::COLOR},
            {"normalMapStrength", ShaderInput::DataType::FLOAT}
        },
        6,
        &sceneRenderPass->getRenderPass()
    ));
    Shared::shaders.push_back(std::make_unique<Shader>(
        "internal/shaders/goochShader.slang", //9
        "internal/shaders/goochShader.slang",
        std::vector<ShaderInput>{
            {"coolColor", ShaderInput::DataType::COLOR},
            {"warmColor", ShaderInput::DataType::COLOR},
            {"outlineColor", ShaderInput::DataType::COLOR},
            {"outlinePower", ShaderInput::DataType::FLOAT},
            {"roughness", ShaderInput::DataType::FLOAT}
        },
        0,
        &sceneRenderPass->getRenderPass()
    ));
}

void Graphics::loadMaterials()
{
    Console::log("Loading materials", "Graphics");
    Shared::materials.reserve(GR_MAX_MATERIAL_COUNT);

    Material _ppMaterial = Material::instantiate(Shared::shaders[0].get());
    _ppMaterial.setValue("exposure", 0.0f);
    _ppMaterial.setValue("gamma", 1.0f);
    _ppMaterial.createShaderInputBuffer();
    _ppMaterial.createDescriptorSet();
    ppMaterial = std::make_unique<Material>(std::move(_ppMaterial));

    Material _imguiMaterial = Material::instantiate(Shared::shaders[1].get());
    _imguiMaterial.setValue("doSRGBTransform", true);
    _imguiMaterial.createShaderInputBuffer();
    _imguiMaterial.createDescriptorSet();
    imguiMaterial = std::make_unique<Material>(std::move(_imguiMaterial));

    Material _outputMaterial = Material::instantiate(Shared::shaders[1].get());
    _outputMaterial.setValue("doSRGBTransform", false);
    _outputMaterial.createShaderInputBuffer();
    _outputMaterial.createDescriptorSet();
    outputMaterial = std::make_unique<Material>(std::move(_outputMaterial));

    Material _outlineMaterial = Material::instantiate(Shared::shaders[2].get());
    _outlineMaterial.createShaderInputBuffer();
    _outlineMaterial.createDescriptorSet();
    outlineMaterial = std::make_unique<Material>(std::move(_outlineMaterial));

    Material _idBufferMaterial = Material::instantiate(Shared::shaders[3].get());
    _idBufferMaterial.setValue("objectID", -2);
    _idBufferMaterial.createShaderInputBuffer();
    _idBufferMaterial.createDescriptorSet();
    idBufferMaterial = std::make_unique<Material>(std::move(_idBufferMaterial));
    
    Material _skybox = Material::instantiate(Shared::shaders[5].get());
    // m1.setValue("color", glm::vec3(0.1f, 0.3f, 0.05f));
    _skybox.setValue("color", Color(0.f, 0.f, 0.f));
    _skybox.createShaderInputBuffer();
    _skybox.createDescriptorSet();

    Material outlineMat = Material::instantiate(Shared::shaders[4].get());
    // m1.setValue("color", glm::vec3(0.1f, 0.3f, 0.05f));
    outlineMat.createShaderInputBuffer();
    outlineMat.createDescriptorSet();
    Shared::materials.emplace_back(std::move(outlineMat));

    Material m1 = Material::instantiate(Shared::shaders[6].get());
    // m1.setValue("color", glm::vec3(0.1f, 0.3f, 0.05f));
    m1.setValue("color", Color(1.f, 0.8f, 0.3f));
    m1.setValue("roughness", 0.4f);
    m1.setValue("metallic", 0.0f);
    m1.createShaderInputBuffer();
    m1.createDescriptorSet();
    Shared::materials.emplace_back(std::move(m1)); // Should probably be done in instantiate

    Material m3 = Material::instantiate(Shared::shaders[7].get());
    // m1.setValue("color", glm::vec3(0.1f, 0.3f, 0.05f));
    m3.setValue("color", Color(0.f, 0.f, 0.f));
    m3.createShaderInputBuffer();
    m3.createDescriptorSet();
    Shared::materials.emplace_back(std::move(m3)); // Should probably be done in instantiate

    Material m4 = Material::instantiate(Shared::shaders[8].get());
    m4.setValue("color", Color(1.f, 1.f, 1.f));
    m4.setValue("normalMapStrength", 1.0f);
    m4.setTexture(0, textures[0].get()); // Albedo
    m4.setTexture(1, textures[1].get()); // Roughness
    m4.setTexture(2, textures[2].get()); // Metallic
    m4.setTexture(3, textures[3].get()); // Specular
    m4.setTexture(4, textures[4].get()); // Normal
    m4.setTexture(5, textures[5].get()); // Skybox (TEMPORARY)
    m4.createShaderInputBuffer();
    m4.createDescriptorSet();
    Shared::materials.emplace_back(std::move(m4));

    Material m2 = Material::instantiate(Shared::shaders[9].get());
    m2.setValue("coolColor", Color("#47376FFF"));
    m2.setValue("warmColor", Color("#FFCC3DFF"));
    m2.setValue("outlineColor", Color("#424242FF"));
    m2.setValue("outlinePower", 4.0f);
    m2.setValue("roughness", 0.6f);
    m2.createShaderInputBuffer();
    m2.createDescriptorSet();
    Shared::materials.emplace_back(std::move(m2)); // Should probably be done in instantiate
    Shared::materials.emplace_back(std::move(_skybox));
}

void Graphics::bindCameraDescriptor(FrameInfo& frameInfo, GraphicsPipeline* pipeline)
{
    std::vector<VkDescriptorSet> descriptorSets = { frameInfo.cameraDescriptorSet };

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        pipeline->getPipelineLayout(), 
        0,
        static_cast<uint32_t>(descriptorSets.size()),
        descriptorSets.data(), 
        0,
        nullptr
    );
}

void Graphics::bindGlobalDescriptor(FrameInfo& frameInfo, GraphicsPipeline* pipeline)
{

    std::vector<VkDescriptorSet> descriptorSets = { frameInfo.globalDescriptorSet };

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer, 
        VK_PIPELINE_BIND_POINT_GRAPHICS, 
        pipeline->getPipelineLayout(), 
        1,
        static_cast<uint32_t>(descriptorSets.size()),
        descriptorSets.data(), 
        0,
        nullptr
    );
}

void Graphics::drawSkybox()
{
    core::Transform tempTransform{};
    tempTransform.setPosition(camera->transform.getPosition());
    drawMesh(skyboxMesh, 5, tempTransform.getTransform());
}

void Graphics::renderMeshes(FrameInfo& frameInfo, const std::vector<MeshRenderData> &renderQueue)
{
    VkCommandBuffer& commandBuffer = frameInfo.commandBuffer;

    // pipelineManager.renderObjects(frameInfo, gameObjects, commandBuffer);
    GraphicsPipeline* prevPipeline = nullptr;
    std::vector<VkDescriptorSet> localDescriptorSets;
    VkPipelineLayout pipelineLayout = nullptr;
    Material::id_t prevMaterial = UINT64_MAX;
    for(const MeshRenderData &renderData : renderQueue)
    {
        const Shader* shader = Shared::materials[renderData.materialIndex].getShader();
        GraphicsPipeline* pipeline = shader->getPipeline();
        uint32_t setIndex = pipeline->getID() + 1;
        if(pipeline != prevPipeline) // Bind camera and global data
        {
            pipeline->bind(frameInfo.commandBuffer);
            pipelineLayout = pipeline->getPipelineLayout();
            bindCameraDescriptor(frameInfo, pipeline);
            bindGlobalDescriptor(frameInfo, pipeline);
            prevPipeline = pipeline;
        }
        localDescriptorSets = { Shared::materials[renderData.materialIndex].getDescriptorSet() };

        if(prevMaterial != renderData.materialIndex) // Bind material info if changed
        {
            Console::debug(std::to_string(renderData.materialIndex));
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
            prevMaterial = renderData.materialIndex;
        }

        PushConstants push{}; // TODO: Instance specific data
        push.objectID = renderData.meshID; // TODO: Change to scene local ID
        vkCmdPushConstants(
            commandBuffer, 
            pipeline->getPipelineLayout(), 
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
            0, 
            sizeof(PushConstants), 
            &push
        );

        std::unique_ptr<GraphicsMesh> &graphicsMesh = graphicsMeshes[renderData.meshID];
        if(graphicsMesh != nullptr)
        {
            graphicsMesh->bind(commandBuffer, renderData.instanceBuffer);
            graphicsMesh->draw(commandBuffer, renderData.transforms.size());
        }
    }
}

void Graphics::renderGameObjectIDs(FrameInfo& frameInfo)
{
    VkCommandBuffer& commandBuffer = frameInfo.commandBuffer;

    // pipelineManager.renderObjects(frameInfo, gameObjects, commandBuffer);
    std::vector<VkDescriptorSet> localDescriptorSets;
    const Shader* shader = Shared::shaders[2].get();
    GraphicsPipeline* pipeline = shader->getPipeline();
    VkPipelineLayout pipelineLayout = pipeline->getPipelineLayout();
    uint32_t setIndex = pipeline->getID() + 1;

    pipeline->bind(frameInfo.commandBuffer);
    bindCameraDescriptor(frameInfo, pipeline);


    for(MeshRenderData &renderData : sceneRenderQueue)
    {
        PushConstants push{};

        push.objectID = renderData.meshID; // TODO: Change to scene local ID
        // Console::debug(std::to_string(push.objectID), "Graphics");
        vkCmdPushConstants(
            commandBuffer, 
            pipelineLayout, 
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
            0, 
            sizeof(PushConstants), 
            &push
        );

        std::unique_ptr<GraphicsMesh> &graphicsMesh = graphicsMeshes[renderData.meshID];
        if(graphicsMesh != nullptr)
        {
            graphicsMesh->bind(commandBuffer, renderData.instanceBuffer);
            graphicsMesh->draw(commandBuffer, renderData.transforms.size());
        }
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
    initInfo.RenderPass = imguiRenderPass->getRenderPass();
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
    Console::log("Reloading Shaders", "Graphics");
    for(std::unique_ptr<Shader>& shader : Shared::shaders)
    {
        shader->reloadShader();
    }

    pipelineManager->reloadPipelines();
}

int Graphics::getClickedObjID(uint32_t x, uint32_t y)
{
    if(!idTexture)
    {
        return -1;
    }

    idTexture->updateOnCPU();

    return idTexture->getPixelInt(x, y);
}

// Mesh management
void Graphics::setGraphicsMesh(const core::Mesh& mesh)
{
    graphicsMeshes[mesh->getInstanceID()] = std::make_unique<GraphicsMesh>(mesh.get());
}

void Graphics::destroyGraphicsMeshes()
{
    graphicsMeshes.clear(); // Destroy all graphicsmeshes
    sceneRenderQueue.clear(); // Ensure no meshes are queued for drawing
}

void Graphics::drawMesh(const core::Mesh& mesh, uint32_t materialIndex, const glm::mat4& transform)
{
    if(!graphicsMeshes.contains(mesh->getInstanceID()))
    {
        Console::log("Mesh " + std::to_string(mesh->getInstanceID()) + " has no GraphicsMesh. Creating one now...", "Graphics");
        setGraphicsMesh(mesh);
    }

    sceneRenderQueue.push_back(MeshRenderData(mesh->getInstanceID(), transform, materialIndex));
}

void Graphics::drawMeshInstanced(const core::Mesh& mesh, uint32_t materialIndex, const std::vector<glm::mat4> &transforms)
{
    if(!graphicsMeshes.contains(mesh->getInstanceID()))
    {
        Console::log("Mesh " + std::to_string(mesh->getInstanceID()) + " has no GraphicsMesh. Creating one now...", "Graphics");
        setGraphicsMesh(mesh);
    }
    if(transforms.size() == 0) return; // No work to do with no transforms

    sceneRenderQueue.push_back(MeshRenderData(mesh->getInstanceID(), transforms, materialIndex));
}

void Graphics::drawMeshOutline(const core::Mesh& mesh, uint32_t materialIndex, const glm::mat4& transform)
{
    if(!graphicsMeshes.contains(mesh->getInstanceID()))
    {
        Console::log("Mesh " + std::to_string(mesh->getInstanceID()) + " has no GraphicsMesh. Creating one now...", "Graphics");
        setGraphicsMesh(mesh);
    }

    outlineRenderQueue.push_back(MeshRenderData(mesh->getInstanceID(), transform, 0));
}

std::unique_ptr<Buffer> Graphics::createInstanceBuffer(const std::vector<glm::mat4>& transforms)
{
    uint32_t instanceCount = transforms.size();
    VkDeviceSize bufferSize = sizeof(transforms[0]) * instanceCount;
    uint32_t instanceSize = sizeof(transforms[0]);

    Buffer stagingBuffer{
        *Shared::device,
        instanceSize,
        instanceCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    };

    stagingBuffer.map();
    stagingBuffer.writeToBuffer((void *)transforms.data());

    std::unique_ptr<Buffer> instanceBuffer = std::make_unique<Buffer>(
        *Shared::device,
        instanceSize,
        instanceCount,
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    Shared::device->copyBuffer(stagingBuffer.getBuffer(), instanceBuffer->getBuffer(), bufferSize);
    return std::move(instanceBuffer);
}

} // namespace graphics