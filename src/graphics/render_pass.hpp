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
        RenderPass();
        ~RenderPass();

        RenderPass(const RenderPass&) = delete; // No copy
        RenderPass &operator=(const RenderPass &) = delete;

        const VkRenderPass getRenderPass() { return renderPass; }
        const VkFramebuffer getFrameBuffer() { return frameBuffer; }
        const VkExtent2D getExtent() { return extent; }
    private:
        VkRenderPass renderPass;
        VkFramebuffer frameBuffer;
        VkExtent2D extent;

        std::vector<std::unique_ptr<Texture>> attachments;
};

} // namespace graphics