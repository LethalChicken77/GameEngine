#include <iostream>
#include <stdexcept>
#include <array>
#include <GLFW/glfw3.h>

#include "engine.hpp"
#include "core/input.hpp"
#include "graphics/mesh.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include "utils/imgui_styles.hpp"

using namespace graphics;

namespace core
{

// struct ObjectUbo
// {

// };

Engine::Engine(graphics::Graphics& _graphics) : graphics(_graphics)
{
    init();
}

Engine::~Engine()
{
    close();
}

void Engine::init()
{
    loadGameObjects();
}

void Engine::close()
{
    // graphics->cleanup();
}

void Engine::update(double deltaTime)
{
    glm::vec3 forward = camera.transform.forward();
    forward.y = 0;
    forward = glm::normalize(forward);
    forward *= glm::sign(camera.transform.up().y);
    glm::vec3 right = camera.transform.right();
    float movementSpeed = 10.f;
    if(core::Input::getKey(GLFW_KEY_A))
    {
        camera.transform.position -= movementSpeed * (float)deltaTime * right;
    }
    if(core::Input::getKey(GLFW_KEY_D))
    {
        camera.transform.position += movementSpeed * (float)deltaTime * right;
    }
    if(core::Input::getKey(GLFW_KEY_W))
    {
        camera.transform.position += movementSpeed * (float)deltaTime * forward;
    }
    if(core::Input::getKey(GLFW_KEY_S))
    {
        camera.transform.position -= movementSpeed * (float)deltaTime * forward;
    }

    if(core::Input::getKey(GLFW_KEY_SPACE))
    {
        camera.transform.position.y += 10.f * deltaTime;
    }
    if(core::Input::getKey(GLFW_KEY_LEFT_SHIFT))
    {
        camera.transform.position.y -= 10.f * deltaTime;
    }

    if(core::Input::getButton(GLFW_MOUSE_BUTTON_RIGHT))
    {
        // std::cout << "Mouse position: " << core::Input::getMousePosition().x << ", " << core::Input::getMousePosition().y << std::endl;
        glm::vec2 mouseDelta = -core::Input::getMouseDelta();
        camera.transform.rotation.y += -mouseDelta.x * 0.016f;
        camera.transform.rotation.x += -mouseDelta.y * 0.016f;
    }

    int counter = 0;
    for(std::unique_ptr<core::GameObject_t> &obj : gameObjects)
    {
        if(obj->get_id() == 0)
            obj->transform.position = camera.transform.position; // Skybox
        // if(obj.get_id() == 2) break;
        // if(obj.get_id() == 1)
        // {
        //     obj.transform.rotation.y += 0.5f * deltaTime;
        // }
        // obj.transform.position = glm::vec3(glm::sin(glfwGetTime()) * (counter % 2 ? 1 : -1));
        // obj.transform.position = glm::vec3(glm::sin(glm::radians(324.f)) * (counter % 2 ? 1 : -1));
        // obj.transform.rotation = glm::vec3(glm::radians(glfwGetTime() * 90.0f) * (counter % 2 ? 1 : -1));
        // obj.transform.rotation = glm::vec3(glm::radians(324.f) * (counter % 2 ? 1 : -1));
        // obj.transform.rotation.x = glm::radians(-90.0f);
        counter++;
    }
}



void Engine::run()
{
    // std::cout << "Configuring IMGUI" << std::endl;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& imguiIO = ImGui::GetIO();
    (void)imguiIO;
    // Set style (optional)
    // ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    // Set window rounding
    // style.WindowRounding = 5.0f;
    // style.FrameRounding = 3.0f;
    // style.GrabRounding = 2.0f;

    // Adjust padding and spacing
    // style.WindowPadding = ImVec2(10, 10);
    // style.FramePadding = ImVec2(5, 5);
    // style.ItemSpacing = ImVec2(8, 4);

    // Modify colors
    StyleColorsDarkLinear(&style);


    ImGui_ImplGlfw_InitForVulkan(graphics.getWindow()->getWindow(), true);
    graphics.graphicsInitImgui();


    Input::initializeKeys();


    camera = Camera(
        {0.01f, 1000.0f, 90.0f},
        false
    );
    // camera = Camera(
    //     {0.01f, 100.0f, 20.0f},
    //     true
    // );
    camera.transform.position = glm::vec3(0.0f, 2.0f, -5.0f);
    camera.transform.rotation = glm::vec3(glm::radians(20.0f), 0.0f, 0.0f);

    graphics.setCamera(&camera);


    std::cout << "Entering main loop" << std::endl;
    double time = 0.0f;
    double deltaTime = 0.0f;
    // Main loop
    while (graphics.isOpen()) {
        // Poll for and process events
        glfwPollEvents();
        
        if(core::Input::getKeyDown(GLFW_KEY_R))
        {
            graphics.reloadShaders();
        }

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        Console::drawImGui();

        ImGui::Begin("Material Properties");

        for(Material &mat : Shared::materials)
        {
            mat.drawImGui();
        }
        
        ImGui::End();

        bool imguiHovered = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
        ImGui::Render();
        // Input
        Input::processInput(graphics.getWindow()->getWindow());
        
        if(core::Input::getKeyDown(GLFW_KEY_ESCAPE))
        {
            break;
        }

        update(deltaTime);

        // Render here
        graphics.drawFrame(gameObjects);


        // Update Time
        double oldTime = time;
        time = glfwGetTime();
        deltaTime = time - oldTime;
        // std::cout << "Delta time: " << deltaTime << std::endl;
    }
    close();
}

void Engine::loadGameObjects()
{
    std::cout << "Loading game objects" << std::endl;
    // Triangle
    // std::vector<Mesh::Vertex> vertices {
    //     {{0.0f, -0.5f, 0.0f}, {0.5f, 1.0f}},
    //     {{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f}},
    //     {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f}}
    // };

    std::unique_ptr<GameObject_t> skybox = GameObject_t::Instantiate(); // TODO: Just use a mesh for this
    skybox->mesh = Mesh::createSkybox(100.0f);
    skybox->materialID = 0;


    std::unique_ptr<GameObject_t> obj = GameObject_t::Instantiate();
    std::unique_ptr<GameObject_t> obj2 = GameObject_t::Instantiate();
    std::unique_ptr<GameObject_t> obj3 = GameObject_t::Instantiate();
    // std::cout << "Creating Grid" << std::endl;
    // obj.mesh = Mesh::createGrid(512, 512, {50.0f, 50.0f});
    // obj.materialID = 0;
    obj->mesh = Mesh::loadObj("internal/models/monkey_high_res.obj");
    // obj.mesh->generateNormals();
    // obj.mesh->createBuffers();
    obj->materialID = 1;
    // obj.transform.scale = glm::vec3(0.1f);
    // obj.transform.rotation.x = glm::radians(-90.0f);

    // obj2.mesh = Mesh::loadObj("internal/models/monkey_high_res.obj");
    obj2->mesh = Mesh::createGrid(16,16, {50.0f, 50.0f});
    obj2->materialID = 3;
    obj2->transform.position = glm::vec3(0, -3, 0);

    obj3->mesh = Mesh::loadObj("internal/models/monkey_high_res.obj");
    obj3->materialID = 2;
    obj3->transform.position = glm::vec3(-3, 0, 0);
    // obj3.mesh = Mesh::createSierpinskiPyramid(12.0f, 8);
    // obj3.materialID = 2;
    gameObjects.push_back(std::move(skybox));
    gameObjects.push_back(std::move(obj));
    gameObjects.push_back(std::move(obj2));
    gameObjects.push_back(std::move(obj3));
    std::cout << "Loaded game objects" << std::endl;
}

} // namespace core