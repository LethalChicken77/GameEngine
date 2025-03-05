#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <map>

#include "window.hpp"
#include "graphics_pipeline.hpp"
#include "renderer.hpp"
#include "device.hpp"
#include "buffer.hpp"
#include "descriptors.hpp"
#include "frame_info.hpp"

#include "mesh.hpp"
#include "camera.hpp"
#include "shader.hpp"
#include "material.hpp"
#include "../core/game_object.hpp"
#include "containers.hpp"


namespace graphics
{

#define GR_MAX_MATERIAL_COUNT 128

class Graphics
{
public:
    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    Graphics(const std::string& name, const std::string& engine_name);
    ~Graphics();

    Graphics(const Graphics&) = delete;
    Graphics& operator=(const Graphics&) = delete;

    void init(const std::string& name, const std::string& engine_name);
    void cleanup();
    bool isOpen() const { return window.isOpen(); }

    void drawFrame(std::vector<core::GameObject>& gameObjects);
    void setCamera(Camera* _camera) { camera = _camera; }

    void waitForDevice() { vkDeviceWaitIdle(device.device()); }

    Window *getWindow() { return &window; }
    Device *getDevice() { return &device; }
    void renderGameObjects(FrameInfo& frameInfo, std::vector<core::GameObject>& gameObjects);

    void graphicsInitImgui();

private:
    void loadShaders();
    void loadMaterials();
    void createPipeline();

    static void windowRefreshCallback(GLFWwindow *window);

    VkApplicationInfo appInfo{};

    Window window{WIDTH, HEIGHT, "VEngine"};
    Device device{window};
    Renderer renderer{window, device};
    // Containers containers{&device};

    std::unique_ptr<GraphicsPipeline> graphicsPipeline;
    PipelineConfigInfo configInfo;

    std::vector<std::unique_ptr<Buffer>> cameraUboBuffers;

    std::vector<std::unique_ptr<Shader>> shaders;

    Camera* camera;
};

} // namespace graphics
