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
    alignas(16) glm::mat4 proj;
};

struct LightsUbo
{
    alignas(16) glm::vec3 lightPos;
    alignas(16) glm::vec3 lightColor;
    alignas(4) float lightIntensity;
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

    void beginRenderPass(VkCommandBuffer commandBuffer);
    void endRenderPass(VkCommandBuffer commandBuffer);

    void waitForDevice() { vkDeviceWaitIdle(device.device()); }

    // Getters
    bool isFrameInProgress() const { return frameInProgress; }
    VkCommandBuffer getCurrentCommandBuffer() const 
    { 
        assert(frameInProgress && "Cannot get command buffer as frame is not in progress");
        return commandBuffers[currentFrameIndex]; 
    }
    VkRenderPass getRenderPass() const { return swapChain->getRenderPass(); }
    VkRenderPass getImGuiRenderPass() const { return swapChain->getImGuiRenderPass(); }
    VkExtent2D getExtent() const { return swapChain->getSwapChainExtent(); }
    uint32_t getFrameIndex() const 
    { 
        assert(frameInProgress && "Cannot get frame index as frame is not in progress");
        return currentFrameIndex; 
    }

    static void windowRefreshCallback(GLFWwindow *window);

    glm::vec3 clearColor{0.04f, 0.08f, 0.2f};

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

    uint32_t currentImageIndex = 0;
    uint32_t currentFrameIndex = 0;
    bool frameInProgress = false;
};

} // namespace graphics
