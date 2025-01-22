#include <iostream>
#include <stdexcept>
#include <array>
#include <GLFW/glfw3.h>

#include "renderer.hpp"
#include "mesh.hpp"
#include "../core/input.hpp"

namespace graphics
{

// struct ObjectUbo
// {

// };

Renderer::Renderer(Window& _window, Device& _device) : window(_window), device(_device)
{
    recreateSwapChain();
    createCommandBuffers();
}

Renderer::~Renderer() { freeCommandBuffers(); }


VkCommandBuffer Renderer::startFrame()
{
    assert(!frameInProgress && "Can't start new frame while one is still in progress");

    VkResult result = swapChain->acquireNextImage(&currentImageIndex);
    if(result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        recreateSwapChain();
        return NULL;
    }

    if(result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("Failed to acquire next image");
    }

    frameInProgress = true;

    VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if(vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to begin recording command buffers!");
    }

    return commandBuffer;
}

void Renderer::endFrame()
{
    assert(frameInProgress && "Can't end frame that hasn't been started");

    VkCommandBuffer commandBuffer = getCurrentCommandBuffer();
    if(vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to record command buffer!");
    }

    VkResult result = swapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
    if(result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window.windowResized())
    {
        window.resetWindowResizedFlag();
        recreateSwapChain();
    }
    else if(result != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to submit command buffer");
    }

    frameInProgress = false;
    currentFrameIndex = (currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::beginRenderPass(VkCommandBuffer commandBuffer)
{
    assert(frameInProgress && "Can't begin render pass when frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer that isn't current");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = swapChain->getRenderPass();
    renderPassInfo.framebuffer = swapChain->getFrameBuffer(currentImageIndex);

    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapChain->getSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    // clearValues[0].color = {0.1f, 0.2f, 0.4f, 1.0f};
    clearValues[0].color = {0.04f, 0.08f, 0.2f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChain->getSwapChainExtent().width);
    viewport.height = static_cast<float>(swapChain->getSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0, 0}, swapChain->getSwapChainExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderer::endRenderPass(VkCommandBuffer commandBuffer)
{
    assert(frameInProgress && "Can't end render pass when frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer that isn't current");

    vkCmdEndRenderPass(commandBuffer);
}


void Renderer::recreateSwapChain()
{
    VkExtent2D extent = window.getExtent();
    while(extent.width == 0 || extent.height == 0)
    {
        extent = window.getExtent();
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device.device());

    if(swapChain == nullptr)
    {
        std::cout << "Recreating swap chain" << std::endl;
        swapChain = std::make_unique<SwapChain>(device, extent);
    }
    else
    {
        std::shared_ptr<SwapChain> oldSwapChain = std::move(swapChain);
        std::cout << "Resizing swap chain" << std::endl;
        swapChain = std::make_unique<SwapChain>(device, extent, oldSwapChain);

        if(!oldSwapChain->compareSwapFormats(*swapChain.get()))
        {
            // TODO: Handle this better
            throw std::runtime_error("Swap chain image or depth format has changed!");
        }
    }

    // SHeesh
}

void Renderer::createCommandBuffers()
{
    // std::cout << "Creating command buffers" << std::endl;
    commandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    // currentFrameIndex = 0;

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = device.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

    if(vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
}

void Renderer::freeCommandBuffers()
{
    vkFreeCommandBuffers(
        device.device(), 
        device.getCommandPool(), 
        static_cast<uint32_t>(commandBuffers.size()), 
        commandBuffers.data()
    );
    commandBuffers.clear();
}

void Renderer::windowRefreshCallback(GLFWwindow *window)
{
    Renderer *app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
    app->recreateSwapChain();
}

} // namespace graphics