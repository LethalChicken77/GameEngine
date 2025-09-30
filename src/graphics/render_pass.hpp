#pragma once
#include <memory>
#include <vulkan/vulkan.h>
#include <vector>

#include "texture.hpp"

namespace graphics
{

class RenderPass
{
    public:
        RenderPass(VkExtent2D _extent);
        ~RenderPass();

        RenderPass(const RenderPass&) = delete; // No copy
        RenderPass &operator=(const RenderPass &) = delete;

        VkRenderPass &getRenderPass() { return renderPass; }
        const VkFramebuffer getFrameBuffer() { return frameBuffer; }
        const VkExtent2D getExtent() { return extent; }

        void addColorAttachment(VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        void addDepthAttachment(VkImageLayout imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        // TODO: More attachments for other data

        void resetLayouts();
        void resetLayouts(VkCommandBuffer commandBuffer);

        std::unique_ptr<Texture>& getColorTexture(uint32_t index = 0) { return images[index]; }
        std::unique_ptr<Texture>& getDepthTexture() { return depthImage; }

        void create(VkExtent2D _extent);
    private:
        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkFramebuffer frameBuffer = VK_NULL_HANDLE;
        VkExtent2D extent;

        VkImageLayout colorImageLayout;
        VkImageLayout depthImageLayout;

        std::vector<std::unique_ptr<Texture>> images{};
        std::unique_ptr<Texture> depthImage = nullptr;

        void updateImageSizes();
        void createRenderPass();
        void createFrameBuffers();

        VkFormat findDepthFormat();

        void destroyBuffers();
};

} // namespace graphics