#pragma once
#include <string>
#include <vector>
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <memory>

#include "window.hpp"
#include "swap_chain.hpp"
#include "device.hpp"
#include "mesh.hpp"
#include "buffer.hpp"

#include <cassert>

namespace graphics
{

struct CameraUbo
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 invView;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::mat4 viewProj;
};

enum LightType
{
    DIRECTIONAL = 0,
    POINT = 1
};
struct Light
{
    glm::vec3 position;
    int type = DIRECTIONAL;
    glm::vec3 color;
    float intensity;
};
struct GlobalUbo
{
    glm::vec3 ambient;
    int numLights;
    Light lights[128];
};

class Renderer
{
public:
    Renderer(Window& _window, Device& _device);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    VkCommandBuffer startFrame();
    void endFrame();

    void beginRenderPass(VkRenderPass renderPass, VkFramebuffer frameBuffer, VkExtent2D extent, VkClearColorValue clearColor);
    void endRenderPass();

    void waitForDevice() { vkDeviceWaitIdle(device.device()); }

    // Getters
    bool isFrameInProgress() const { return frameInProgress; }
    VkCommandBuffer getCurrentCommandBuffer() const 
    { 
        assert(frameInProgress && "Cannot get command buffer as frame is not in progress");
        return commandBuffers[currentFrameIndex]; 
    }
    VkRenderPass &getSCRenderPass() { return currentRenderPass; }
    // VkRenderPass getImGuiRenderPass() const { return swapChain->getImGuiRenderPass(); }
    VkExtent2D getExtent() const { return swapChain->getSwapChainExtent(); }
    VkFramebuffer getSCFrameBuffer() const { return swapChain->getFrameBuffer(currentImageIndex); }
    uint32_t getFrameIndex() const 
    { 
        assert(frameInProgress && "Cannot get frame index as frame is not in progress");
        return currentFrameIndex; 
    }

    static void windowRefreshCallback(GLFWwindow *window);

private:
    void recordCommandBuffer(int imageIndex);
    void createCommandBuffers();
    void freeCommandBuffers();
    void recreateSwapChain();

    VkApplicationInfo appInfo{};

    Window& window;
    Device& device;
    std::unique_ptr<SwapChain> swapChain;
    std::vector<VkCommandBuffer> commandBuffers;
    VkCommandBuffer currentCommandBuffer;

    VkRenderPass currentRenderPass = nullptr; // Must track in here because swapchain is destroyed and recreated

    uint32_t currentImageIndex = 0;
    uint32_t currentFrameIndex = 0;
    bool frameInProgress = false;
};

} // namespace graphics
