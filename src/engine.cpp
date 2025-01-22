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

    if(core::Input::getButton(GLFW_MOUSE_BUTTON_LEFT))
    {
        // std::cout << "Mouse position: " << core::Input::getMousePosition().x << ", " << core::Input::getMousePosition().y << std::endl;
        glm::vec2 mouseDelta = -core::Input::getMouseDelta();
        camera.transform.rotation.y += -mouseDelta.x * 10.f * deltaTime;
        camera.transform.rotation.x += -mouseDelta.y * 10.f * deltaTime;
    }

    int counter = 0;
    for(GameObject &obj : gameObjects)
    {
        obj.transform.position = glm::vec3(glm::sin(glfwGetTime()) * (counter % 2 ? 1 : -1));
        // obj.transform.position = glm::vec3(glm::sin(324.f) * (counter % 2 ? 1 : -1));
        obj.transform.rotation = glm::vec3(glm::radians(glfwGetTime() * 90.0f) * (counter % 2 ? 1 : -1));
        // obj.transform.rotation = glm::vec3(glm::radians(324.f * 90.f));
        // obj.transform.rotation.x = glm::radians(-90.0f);
        counter++;
    }
}

void Engine::run()
{
    Input::initializeKeys();


    camera = Camera(
        {0.01f, 100.0f, 90.0f},
        false
    );
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
    // Triangle
    // std::vector<Mesh::Vertex> vertices {
    //     {{0.0f, -0.5f, 0.0f}, {0.5f, 1.0f}},
    //     {{0.5f, 0.5f, 0.0f}, {1.0f, 0.0f}},
    //     {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f}}
    // };

    GameObject obj = GameObject::instantiate();
    GameObject obj2 = GameObject::instantiate();
    obj.mesh = Mesh::loadObj(*graphics.getDevice(), "internal/models/monkey_high_res.obj");
    obj2.mesh = Mesh::loadObj(*graphics.getDevice(), "internal/models/monkey_high_res.obj");
    gameObjects.push_back(std::move(obj));
    gameObjects.push_back(std::move(obj2));
}

} // namespace core