#pragma once
#include <memory>
#include <vulkan/vulkan.h>
#include <vector>
#include <map>

#include "graphics/buffers/texture.hpp"
#include "graphics/internal/graphics_pipeline.hpp"
#include "graphics/frame_info.hpp"

namespace graphics
{

class RenderPass
{
    public:
        ~RenderPass();

        RenderPass(const RenderPass&) = delete; // No copy
        RenderPass &operator=(const RenderPass &) = delete;

        VkRenderPass &getRenderPass() { return renderPass; }
        const VkFramebuffer getFrameBuffer() { return frameBuffer; }
        const VkExtent2D getExtent() { return extent; }

        void addColorAttachment(VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        void addColorAttachment(TextureProperties textureProps, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        void addColorAttachment(SamplerProperties samplerProps, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        void addColorAttachment(TextureProperties textureProps, SamplerProperties samplerProps, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        void addDepthAttachment(VkImageLayout imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        // TODO: More attachments for other data

        void resetLayouts();
        void resetLayouts(VkCommandBuffer commandBuffer);

        std::unique_ptr<Texture>& getColorTexture(uint32_t index = 0) { return images[index]; }
        std::unique_ptr<Texture>& getDepthTexture() { return depthImage; }

        void create(VkExtent2D _extent);
    private:
        RenderPass(VkExtent2D initialExtent);
        VkRenderPass renderPass = VK_NULL_HANDLE;
        VkFramebuffer frameBuffer = VK_NULL_HANDLE;
        VkExtent2D extent;

        VkImageLayout colorImageLayout;
        VkImageLayout depthImageLayout;

        std::vector<std::unique_ptr<Texture>> images{};
        std::vector<VkImage> externalImages{};
        std::map<std::string, std::unique_ptr<Texture>&> imageRefs{};
        std::unique_ptr<Texture> depthImage = nullptr;
        GraphicsPipeline* pipeline = nullptr;

        void updateImageSizes();
        void createRenderPass();
        void createFrameBuffers();

        static VkFormat findDepthFormat();

        void destroyBuffers();

        friend class RenderPassBuilder;
};

class RenderPassBuilder
{
    private:
        RenderPass newPass;
    public:
        RenderPassBuilder(VkExtent2D initialExtent);

        RenderPassBuilder &AddExternalColorAttachment(Texture *texture);
        RenderPassBuilder &AddColorAttachment(std::string_view passName, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        RenderPassBuilder &AddColorAttachment(std::string_view passName, TextureProperties textureProps, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        RenderPassBuilder &AddColorAttachment(std::string_view passName, SamplerProperties samplerProps, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        RenderPassBuilder &AddColorAttachment(std::string_view passName, TextureProperties textureProps, SamplerProperties samplerProps, VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB, VkImageLayout imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        
        RenderPassBuilder &AddDepthAttachment(VkImageLayout imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        
        RenderPassBuilder &SetDrawFunction(void (*func)(FrameInfo));

        std::unique_ptr<RenderPass> Build();
};

} // namespace graphics