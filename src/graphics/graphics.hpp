#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "window.hpp"
#include "graphics_pipeline.hpp"
#include "renderer.hpp"
#include "device.hpp"
#include "buffer.hpp"
#include "descriptors.hpp"
#include "frame_info.hpp"

#include "mesh.hpp"
#include "camera.hpp"
#include "../core/game_object.hpp"

namespace graphics
{

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

private:
    void loadGameObjects();
    void createPipelineLayout();
    void createPipeline();

    static void windowRefreshCallback(GLFWwindow *window);

    VkInstance instance;
    VkApplicationInfo appInfo{};

    Window window{WIDTH, HEIGHT, "VEngine"};
    Device device{window};
    Renderer renderer{window, device};

    std::unique_ptr<GraphicsPipeline> graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    PipelineConfigInfo configInfo;

    // Declaration order matters
    std::unique_ptr<DescriptorPool> globalPool{};
    std::unique_ptr<DescriptorSetLayout> globalSetLayout{};
    std::vector<VkDescriptorSet> globalDescriptorSets{};

    std::vector<std::unique_ptr<Buffer>> cameraUboBuffers;

    Camera* camera;
};

} // namespace graphics
