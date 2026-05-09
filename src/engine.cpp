#include <iostream>
#include <stdexcept>
#include <array>
#include <GLFW/glfw3.h>

#include "engine.hpp"
#include "modules.hpp"
#include "core/input.hpp"
#include "graphics/resources/graphics_mesh.hpp"

#include "graphics/api/resources/shader.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_INTRINSICS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "utils/imgui_styles.hpp"
#include "arena_allocator.hpp"
#include "pool_allocator.hpp"
#include "analysis/rolling_average.hpp"

using namespace graphics;

namespace core
{

// struct ObjectUbo
// {

// };

Engine::Engine()
{
    gameData = std::make_unique<GameData>();
    init();
}

Engine::~Engine()
{
    gameData.reset();
    close();
}

void Engine::init()
{
    gameData->skyboxMesh = Mesh::createSkybox(10);
    graphicsModule.RegisterMesh(*gameData->skyboxMesh);

    // ShaderAsset *testShader = AssetManager::LoadAsset<ShaderAsset>("internal/shaders/test_shader.slang");
    // testShader->LoadData();
    // graphics::Shader* ts = gameData->shaderPool.New(testShader, testShader);
    ShaderAsset *basicShader = AssetManager::LoadAsset<ShaderAsset>("internal/shaders/basicShader.slang");
    ShaderAsset *goochShader = AssetManager::LoadAsset<ShaderAsset>("internal/shaders/goochShader.slang");
    ShaderAsset *pbrShader = AssetManager::LoadAsset<ShaderAsset>("internal/shaders/PBR.slang");
    ShaderAsset *skyboxShader = AssetManager::LoadAsset<ShaderAsset>("internal/shaders/skybox.slang");

    basicShader->LoadData();
    goochShader->LoadData();
    pbrShader->LoadData();
    skyboxShader->LoadData();

    // Shader shader = Shader(testShader, testShader);
    // This is just to test memory pools, this will be replaced with a proper resource management system
    shader = gameData->shaderPool.New(basicShader, basicShader);
    gShader = gameData->shaderPool.New(goochShader, goochShader);
    sbShader = gameData->shaderPool.New(skyboxShader, skyboxShader);
    pbr = gameData->shaderPool.New(pbrShader, pbrShader);
    sbShader->properties.depthWrite = DepthWrite::Disabled;
    sbShader->Update();
    // Shader* pbr = shaderPool.New(pbrShader, pbrShader);


    std::unique_ptr<TextureData> texa = TextureData::LoadFromFile("./internal/textures/worn_tile_floor/worn_tile_floor_diff_1k.jpg");
    std::unique_ptr<TextureData> texr = TextureData::LoadFromFileEXR("./internal/textures/worn_tile_floor/worn_tile_floor_rough_1k.exr");
    std::unique_ptr<TextureData> texm = TextureData::LoadFromFileEXR("./internal/textures/defaults/default_metal.exr");
    std::unique_ptr<TextureData> texs = TextureData::LoadFromFileEXR("./internal/textures/defaults/default_spec.exr");
    std::unique_ptr<TextureData> texn = TextureData::LoadFromFileEXR("./internal/textures/worn_tile_floor/worn_tile_floor_nor_gl_1k.exr");
    // std::unique_ptr<TextureData> texa = TextureData::LoadFromFile("./internal/textures/rocky_terrain_02/rocky_terrain_02_diff_4k.jpg");
    // // std::unique_ptr<TextureData> texr = TextureData::LoadFromFile("./internal/textures/rocky_terrain_02/rocky_terrain_02_rough_512.png");
    // std::unique_ptr<TextureData> texr = TextureData::LoadFromFileEXR("./internal/textures/rocky_terrain_02/rocky_terrain_02_rough_4k.exr");
    // std::unique_ptr<TextureData> texm = TextureData::LoadFromFileEXR("./internal/textures/defaults/default_metal.exr");
    // std::unique_ptr<TextureData> texs = TextureData::LoadFromFileEXR("./internal/textures/rocky_terrain_02/rocky_terrain_02_spec_4k.exr");
    // std::unique_ptr<TextureData> texn = TextureData::LoadFromFileEXR("./internal/textures/rocky_terrain_02/rocky_terrain_02_nor_gl_4k.exr");

    TextureHandle thA{};
    TextureHandle thR{};
    TextureHandle thM{};
    TextureHandle thS{};
    TextureHandle thN{};

    TextureData* texaP = gameData->textures.New(std::move(*texa));
    TextureData* texrP = gameData->textures.New(std::move(*texr));
    TextureData* texmP = gameData->textures.New(std::move(*texm));
    TextureData* texsP = gameData->textures.New(std::move(*texs));
    TextureData* texnP = gameData->textures.New(std::move(*texn));

    if(texa != nullptr) thA = graphicsModule.RegisterTexture(*texaP);
    if(texr != nullptr) thR = graphicsModule.RegisterTexture(*texrP);
    if(texm != nullptr) thM = graphicsModule.RegisterTexture(*texmP);
    if(texs != nullptr) thS = graphicsModule.RegisterTexture(*texsP);
    if(texn != nullptr) thN = graphicsModule.RegisterTexture(*texnP);

    graphics::Material *yella = gameData->materials.New(shader, "Yeller");
    graphics::Material *blue = gameData->materials.New(shader, "Bloo");
    graphics::Material *pbrMat = gameData->materials.New(pbr, "Justin Pbr");
    graphics::Material *skyboxMat = gameData->materials.New(sbShader, "Skybox");
    // skyboxMat->UpdateValues();
    int64_t testVal = 69;
    // mat.SetInt("test", testVal);
    yella->SetColor3("color", Color(1.f, 0.8f, 0.3f));
    yella->SetFloat("roughness", 0.2f);
    yella->SetFloat("metallic", 0.0f);

    blue->SetColor3("color", Color(0.2f, 0.5f, 0.8f));
    blue->SetFloat("roughness", 0.7f);
    blue->SetFloat("metallic", 0.0f);
    // blue->SetVector("coolColor", glm::vec4(0.15f, 0.2f, 0.4f, 1.0f));
    // blue->SetVector("warmColor", glm::vec4(0.9f, 0.9f, 0.8f, 1.0f));
    // blue->SetVector("outlineColor", glm::vec4(0.1f, 0.05f, 0.0f, 1.0f));
    // blue->SetFloat("outlinePower", 4.0f);
    // blue->SetFloat("roughness", 0.5f);
    // blue->name = "Gooch";

    // pbrMat->SetVector("color", glm::vec4(0.1f, 0.5f, 0.1f, 1.0f));
    pbrMat->SetVector("color", glm::vec4(1.f, 1.f, 1.f, 1.f));
    pbrMat->SetFloat("normalMapStrength", 1.f);
    pbrMat->SetVector("UVOffset", glm::vec2(0,0));
    pbrMat->SetVector("UVScale", glm::vec2(250,250));
    // gameData->materials[2].SetFloat("roughness", 0.8f);
    // gameData->materials[2].SetFloat("metallic", 0.0f);
    pbrMat->SetTexture("albedoMap", thA);
    pbrMat->SetTexture("roughnessMap", thR);
    pbrMat->SetTexture("metallicMap", thM);
    pbrMat->SetTexture("specularMap", thS);
    pbrMat->SetTexture("normalMap", thN);

    gameData->skyboxMaterial = skyboxMat;
    // mat2.SetVector("color", glm::vec4(1));
    // mat2.SetFloat("normalMapStrength", 1.0f);
    // mat2.SetTexture("albedoMap", );
    // mat2.SetTexture("roughnessMap", );
    // mat2.SetTexture("metallicMap", );
    // mat2.SetTexture("normalMap", );

    graphicsModule.GraphicsInitImgui();
    // ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;


    Input::initializeKeys();


    CameraProperties camProps{};
    camProps.near = 0.01f;
    camProps.far = INFINITY;
    camera = Camera(camProps);
    // camera = Camera(
    //     {0.01f, 100.0f, 20.0f},
    //     true
    // );
    cameraTransform.setPosition(glm::vec3(0.0f, 2.0f, 5.0f));
    cameraTransform.setRotation(glm::vec3(glm::radians(20.0f), glm::radians(-180.0f), 0.0f));

    scene = Scene("Test");
    scene->loadScene();
}

void Engine::close()
{
    // graphicsModule.cleanup();
}

void Engine::update(double deltaTime)
{
    glm::vec3 forward = cameraTransform.forward();
    forward.y = 0;
    forward = glm::normalize(forward);
    forward *= glm::sign(cameraTransform.up().y);
    glm::vec3 right = cameraTransform.right();
    float movementSpeed = 10.f;
    if(core::Input::getKey(GLFW_KEY_LEFT_CONTROL))
    {
        movementSpeed *= 10.f;
    }
    if(core::Input::getKey(GLFW_KEY_A))
    {
        cameraTransform.addPosition(-movementSpeed * (float)deltaTime * right);
    }
    if(core::Input::getKey(GLFW_KEY_D))
    {
        cameraTransform.addPosition(movementSpeed * (float)deltaTime * right);
    }
    if(core::Input::getKey(GLFW_KEY_W))
    {
        cameraTransform.addPosition(movementSpeed * (float)deltaTime * forward);
    }
    if(core::Input::getKey(GLFW_KEY_S))
    {
        cameraTransform.addPosition(-movementSpeed * (float)deltaTime * forward);
    }
    if(core::Input::getKey(GLFW_KEY_SPACE))
    {
        cameraTransform.addPosition(glm::vec3(0, movementSpeed * deltaTime, 0));
    }
    if(core::Input::getKey(GLFW_KEY_LEFT_SHIFT))
    {
        cameraTransform.addPosition(glm::vec3(0, -movementSpeed * deltaTime, 0));
    }

    if(core::Input::getKeyDown(GLFW_KEY_R))
    {
        // TODO: Make a better system for this
        shader->Compile();
        gShader->Compile();
        sbShader->Compile();
        pbr->Compile();
    }

    if(core::Input::getKeyDown(GLFW_KEY_F10))
    {
        graphicsModule.GetWindow()->ToggleFullscreenBorderless();
    }
    if(core::Input::getKeyDown(GLFW_KEY_F11))
    {
        graphicsModule.GetWindow()->ToggleFullscreen();
    }

    // TODO: Reimplement object selection
    // if(core::Input::getButtonDown(GLFW_MOUSE_BUTTON_LEFT) && !core::Input::getButton(GLFW_MOUSE_BUTTON_RIGHT))
    // {
    //     glm::vec2 mousePos = core::Input::getMousePosition();
    //     int mouseXPos = mousePos.x;
    //     int mouseYPos = mousePos.y;
    //     Console::debug(std::to_string(mouseXPos) + " " + std::to_string(mouseYPos));
    //     Console::debug(std::to_string(graphicsModule.getClickedObjID(mouseXPos, mouseYPos)));
    //     scene->selectedObject = graphicsModule.getClickedObjID(mouseXPos, mouseYPos);
    // }

    if(core::Input::getButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
    {
        glfwSetInputMode(graphicsModule.GetGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if(core::Input::getButton(GLFW_MOUSE_BUTTON_RIGHT))
    {
        // std::cout << "Mouse position: " << core::Input::getMousePosition().x << ", " << core::Input::getMousePosition().y << std::endl;
        glm::vec2 mouseDelta = -core::Input::getMouseDelta();
        mouseDelta = glm::pow(glm::abs(mouseDelta), glm::vec2(1.1f)) * glm::vec2(glm::sign(mouseDelta.x), glm::sign(mouseDelta.y));
        cameraTransform.rotateYaw(-mouseDelta.x * 0.016f * 0.1f, false);
        cameraTransform.rotatePitch(-mouseDelta.y * 0.016f * 0.1f, true);
    }
    if(core::Input::getButtonUp(GLFW_MOUSE_BUTTON_RIGHT))
    {
        glfwSetInputMode(graphicsModule.GetGLFWWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    scene->update(deltaTime);
}

void Engine::run()
{
    // camera.transform.parent = &scene->getGameObjects()[0]->transform;

    // graphicsModule.SetCamera(&camera);

    VkDescriptorSet viewPortDS = nullptr;

    analysis::RollingAverage<float> averageFrameTime{120};
    std::cout << "Entering main loop" << std::endl;
    double time = 0.0f;
    double deltaTime = 0.0f;
    // Main loop
    while (graphicsModule.IsOpen()) {
        // Poll for and process events
        glfwPollEvents();
        
        // TODO: Reimplement
        // if(core::Input::getKeyDown(GLFW_KEY_R))
        // {
        //     graphicsModule.ReloadShaders();
        // }


        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);

        // ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0,0));
        // ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
        // ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoDecoration);
        // ImVec2 size = ImGui::GetContentRegionAvail();
        // graphicsModule.viewportSize = VkExtent2D{(uint32_t)size.x, (uint32_t)size.y};
        // graphicsModule.updateExtent();
        // viewPortDS = graphicsModule.getViewportDescriptorSet();
        // if(viewPortDS != nullptr)
        // {
        //     ImGui::Image((void*)reinterpret_cast<uintptr_t>(viewPortDS), size);
        // }
        // ImGui::End();
        // ImGui::PopStyleVar(2);

        Console::drawImGui();
        // ObjectManager::drawImGui();
        drawPerformancePanel(deltaTime, averageFrameTime.GetAverage());
        averageFrameTime.PushValue(deltaTime);

        ImGui::Begin("Material Properties");

        gameData->materials.Get(2)->DrawImGui();
        // for(Material &mat : Shared::materials)
        // {
        //     mat.drawImGui();
        // }
        
        ImGui::End();


        bool imguiHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
        ImGui::Render();
        // ImGui::UpdatePlatformWindows();
        // ImGui::RenderPlatformWindowsDefault();
        // Input
        Input::processInput(graphicsModule.GetGLFWWindow());
        
        if(core::Input::getKeyDown(GLFW_KEY_ESCAPE))
        {
            break;
        }

        // Update Time
        double oldTime = time;
        time = glfwGetTime();
        deltaTime = time - oldTime;
        
        update(deltaTime);

        Transform skyboxTransform{};
        skyboxTransform.setPosition(cameraTransform.getPosition());
        graphicsModule.DrawMesh(gameData->skyboxMesh, *gameData->skyboxMaterial, skyboxTransform.getTransform(), -1); // Draw skybox
        scene->drawScene();
        // Render here
        camera.setAspectRatio(graphicsModule.GetAspectRatio());
        camera.SetTransform(cameraTransform.getTransform());
        graphicsModule.SetCamera(camera);
        graphicsModule.DrawFrame();


        // std::cout << "Delta time: " << deltaTime << std::endl;
    }
    close();
}

void Engine::drawPerformancePanel(float deltaTime, float smoothedDeltaTime)
{
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(
        ImVec2(vp->WorkPos.x + vp->WorkSize.x, vp->WorkPos.y),
        ImGuiCond_Always,
        ImVec2(1.0f, 0.0f) // pivot: top-right
    );

    ImGui::SetNextWindowSize(ImVec2(0, 0), ImGuiCond_Once);
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove;
    ImGui::Begin("Performance", nullptr, flags);

    float fps = 1/smoothedDeltaTime;
    ImGui::Text("%.1f FPS\n%.2f ms", fps, smoothedDeltaTime * 1000);
    float rawfps = 1/deltaTime;
    ImGui::Text("%.1f FPS (raw)\n%.2f ms (raw)      ", rawfps, deltaTime * 1000);

    ImGui::End();
}

} // namespace core