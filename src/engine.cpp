#include <iostream>
#include <stdexcept>
#include <array>
#include <GLFW/glfw3.h>

#include "engine.hpp"
#include "core/input.hpp"
#include "graphics/graphics_mesh.hpp"

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
    scene = Scene(ObjectManager::Instantiate<Scene_t>());
    scene->loadScene();
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

    if(core::Input::getButtonDown(GLFW_MOUSE_BUTTON_LEFT) && !core::Input::getButton(GLFW_MOUSE_BUTTON_RIGHT))
    {
        glm::vec2 mousePos = core::Input::getMousePosition();
        int mouseXPos = mousePos.x;
        int mouseYPos = mousePos.y;
        Console::debug(std::to_string(mouseXPos) + " " + std::to_string(mouseYPos));
        Console::debug(std::to_string(graphics.getClickedObjID(mouseXPos, mouseYPos)));
    }

    if(core::Input::getButtonDown(GLFW_MOUSE_BUTTON_RIGHT))
    {
        glfwSetInputMode(graphics.getWindow()->getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    if(core::Input::getButton(GLFW_MOUSE_BUTTON_RIGHT))
    {
        // std::cout << "Mouse position: " << core::Input::getMousePosition().x << ", " << core::Input::getMousePosition().y << std::endl;
        glm::vec2 mouseDelta = -core::Input::getMouseDelta();
        mouseDelta = glm::pow(glm::abs(mouseDelta), glm::vec2(1.1f)) * glm::vec2(glm::sign(mouseDelta.x), glm::sign(mouseDelta.y));
        camera.transform.rotation.y += -mouseDelta.x * 0.016f * 0.1f;
        camera.transform.rotation.x += -mouseDelta.y * 0.016f * 0.1f;
    }
    if(core::Input::getButtonUp(GLFW_MOUSE_BUTTON_RIGHT))
    {
        glfwSetInputMode(graphics.getWindow()->getWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    scene->update(deltaTime);
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
    // StyleColorsDarkLinear(&style); // Done in post processing now


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
        graphics.drawFrame(scene);


        // Update Time
        double oldTime = time;
        time = glfwGetTime();
        deltaTime = time - oldTime;
        // std::cout << "Delta time: " << deltaTime << std::endl;
    }
    close();
}

} // namespace core