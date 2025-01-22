#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "graphics/window.hpp"
#include "graphics/graphics.hpp"
#include "graphics/camera.hpp"
#include "core/game_object.hpp"
#include "utils/debug.hpp"

namespace core
{

class Engine
{
public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    Engine(graphics::Graphics& _graphics);
    ~Engine();

    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;

    void init();
    void close();
    bool isOpen() const { return graphics.getWindow()->isOpen(); }

    void run();

private:
    void loadGameObjects();
    void createPipelineLayout();
    void createPipeline();

    void update(double deltaTime);

    static void windowRefreshCallback(GLFWwindow *window);

    VkInstance instance;
    VkApplicationInfo appInfo{};

    graphics::Graphics& graphics;

    graphics::Camera camera;
    std::vector<core::GameObject> gameObjects;
};

} // namespace core
